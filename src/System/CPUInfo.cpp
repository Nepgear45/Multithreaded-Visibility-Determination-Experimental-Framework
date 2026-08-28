#include "CPUInfo.h"

#include <array>
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32

#include <windows.h>
#include <intrin.h>

#elif defined(__linux__)

#include <cpuid.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>

#endif


namespace
{
    // CPUID
    void CPUID
    (
        int output[4],
        int function,
        int subFunction = 0
    )
    {
    #ifdef _WIN32

        __cpuidex
        (
            output,
            function,
            subFunction
        );

    #elif defined(__linux__)

        unsigned int eax = 0;
        unsigned int ebx = 0;
        unsigned int ecx = 0;
        unsigned int edx = 0;

        __cpuid_count
        (
            function,
            subFunction,
            eax,
            ebx,
            ecx,
            edx
        );

        output[0] = static_cast<int>(eax);
        output[1] = static_cast<int>(ebx);
        output[2] = static_cast<int>(ecx);
        output[3] = static_cast<int>(edx);

    #endif
    }

    // CPU Vendor
    std::string GetVendor()
    {
        int registers[4]{};

        CPUID(registers, 0);

        char vendor[13]{};

        std::memcpy
        (
            vendor + 0,
            &registers[1],
            4
        );

        std::memcpy
        (
            vendor + 4,
            &registers[3],
            4
        );

        std::memcpy
        (
            vendor + 8,
            &registers[2],
            4
        );

        return vendor;
    }

    // CPU Retail / Brand Name
    std::string GetRetailName()
    {
        int registers[4]{};

        CPUID
        (
            registers,
            0x80000000
        );

        const unsigned int maxExtendedLeaf = static_cast<unsigned int>(registers[0]);

        if (maxExtendedLeaf < 0x80000004) return "Unknown";

        std::array<char, 49> brand{};

        for (int i = 0; i < 3; ++i)
        {
            CPUID
            (
                registers,
                0x80000002 + i
            );

            std::memcpy
            (
                brand.data() + (i * 16),
                registers,
                16
            );
        }

        std::string result = brand.data();

        const size_t firstCharacter = result.find_first_not_of(' ');

        if (firstCharacter != std::string::npos) result.erase(0, firstCharacter);

        return result;
    }


#ifdef _WIN32

    // Windows CPU Topology
    struct Topology
    {
        unsigned int physicalCores = 0;
        unsigned int logicalProcessors = 0;

        unsigned int performanceCores = 0;
        unsigned int efficiencyCores = 0;

        bool heterogeneous = false;
    };


    Topology GetTopology()
    {
        Topology topology{};

        DWORD length = 0;

        GetLogicalProcessorInformationEx
        (
            RelationProcessorCore,
            nullptr,
            &length
        );

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) return topology;

        std::vector<unsigned char> buffer(length);

        auto* information = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());

        if (!GetLogicalProcessorInformationEx(RelationProcessorCore, information, &length)) return topology;

        std::set<BYTE> efficiencyClasses;

        DWORD offset = 0;

        while (offset < length)
        {
            auto* entry = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data() + offset);

            if (entry->Relationship == RelationProcessorCore)
            {
                ++topology.physicalCores;
                efficiencyClasses.insert(entry->Processor.EfficiencyClass);
            }

            offset += entry->Size;
        }

        // Logical Processor Count
        topology.logicalProcessors = std::thread::hardware_concurrency();

        // Detect Heterogeneous Core Classes
        topology.heterogeneous = efficiencyClasses.size() > 1;

        // P / E Core Counts
        if (topology.heterogeneous)
        {
            const BYTE lowestClass = *efficiencyClasses.begin();
            const BYTE highestClass = *efficiencyClasses.rbegin();

            offset = 0;

            while (offset < length)
            {
                auto* entry = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data() + offset);

                if (entry->Relationship == RelationProcessorCore)
                {
                    const BYTE efficiencyClass = entry->Processor.EfficiencyClass;
                    if (efficiencyClass == highestClass) ++topology.performanceCores;
                    else if (efficiencyClass == lowestClass) ++topology.efficiencyCores;
                }
                offset += entry->Size;
            }
        }
        return topology;
    }

#endif


#ifdef __linux__

    // Linux CPU Topology
    struct Topology
    {
        unsigned int physicalCores = 0;
        unsigned int logicalProcessors = 0;

        unsigned int performanceCores = 0;
        unsigned int efficiencyCores = 0;

        bool heterogeneous = false;
    };


    int ReadIntegerFile(const std::filesystem::path& path)
    {
        std::ifstream file(path);
        int value = -1;
        if (file) file >> value;
        return value;
    }


    Topology GetTopology()
    {
        Topology topology{};

        const long logicalCount = sysconf(_SC_NPROCESSORS_ONLN);

        if (logicalCount > 0) topology.logicalProcessors = static_cast<unsigned int>(logicalCount);

        // A physical core is uniquely identified by:
        // package ID + core ID

        std::set<std::pair<int, int>> physicalCores;

        namespace fs = std::filesystem;
        const fs::path cpuRoot = "/sys/devices/system/cpu";

        if (fs::exists(cpuRoot))
        {
            for (const auto& entry : fs::directory_iterator(cpuRoot))
            {
                if (!entry.is_directory()) continue;

                const std::string name = entry.path().filename().string();

                if (name.size() < 4 || name.rfind("cpu", 0) != 0) continue;

                bool numeric = true;

                for (size_t i = 3; i < name.size(); ++i)
                {
                    if (!std::isdigit(static_cast<unsigned char>(name[i])))
                    {
                        numeric = false;
                        break;
                    }
                }

                if (!numeric) continue;

                const fs::path topologyPath = entry.path() / "topology";

                const int packageID = ReadIntegerFile(topologyPath / "physical_package_id");
                const int coreID = ReadIntegerFile(topologyPath / "core_id");


                if (packageID >= 0 && coreID >= 0) physicalCores.insert({ packageID, coreID });
            }
        }
        topology.physicalCores = static_cast<unsigned int>(physicalCores.size());
        return topology;
    }
#endif
}


void CPUInfo::Print()
{
    const std::string retailName = GetRetailName();
    const std::string vendor = GetVendor();
    const Topology topology = GetTopology();

    std::cout << "\nCPU Information\n";
    std::cout << "Retail Name: " << retailName << '\n';
    std::cout << "Vendor: " << vendor << '\n';
    std::cout << "Physical Cores: " << topology.physicalCores << '\n';

    if (topology.heterogeneous)
    {
        std::cout << "Performance Cores (P-Cores): " << topology.performanceCores << '\n';
        std::cout << "Efficiency Cores (E-Cores): " << topology.efficiencyCores << '\n';
    }
    else std::cout << "Core Architecture: Homogeneous" << '\n';

    std::cout << "Logical Processors: " << topology.logicalProcessors << '\n';
}