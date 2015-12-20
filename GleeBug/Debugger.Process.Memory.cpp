#include "Debugger.Process.h"

namespace GleeBug
{
    bool ProcessInfo::MemRead(ptr address, void* buffer, ptr size, ptr* bytesRead) const
    {
        return !!ReadProcessMemory(this->hProcess, reinterpret_cast<const void*>(address), buffer, size, nullptr);
    }

    bool ProcessInfo::MemReadSafe(ptr address, void* buffer, ptr size, ptr* bytesRead) const
    {
        if (!MemRead(address, buffer, size))
            return false;

        //choose the filter method that has the lowest cost
        auto start = address;
        auto end = start + size;
        if (size > breakpoints.size())
        {
            for (const auto & breakpoint : breakpoints)
            {
                if (breakpoint.first.first != BreakpointType::Software)
                    continue;
                const auto & info = breakpoint.second;
                auto curAddress = info.address;
                for (ptr j = 0; j < info.internal.software.size; j++)
                {
                    if (curAddress + j >= start && curAddress + j < end)
                        ((uint8*)buffer)[curAddress + j - start] = info.internal.software.oldbytes[j];
                }
            }
        }
        else
        {
            for (ptr i = start; i < end; i++)
            {
                auto found = softwareBreakpointReferences.find(i);
                if (found == softwareBreakpointReferences.end())
                    continue;
                const auto & info = found->second->second;
                auto curAddress = info.address;
                for (ptr j = 0; j < info.internal.software.size && i < end; j++, i++)
                {
                    if (curAddress + j >= start && curAddress + j < end)
                        ((uint8*)buffer)[curAddress + j - start] = info.internal.software.oldbytes[j];
                }
                i += info.internal.software.size - 1;
            }
        }

        return true;
    }

    bool ProcessInfo::MemWrite(ptr address, const void* buffer, ptr size, ptr* bytesWritten)
    {
        return !!WriteProcessMemory(this->hProcess, reinterpret_cast<void*>(address), buffer, size, nullptr);
    }

    bool ProcessInfo::MemWriteSafe(ptr address, const void* buffer, ptr size, ptr* bytesWritten)
    {
        return false;
    }

    bool ProcessInfo::MemIsValidPtr(ptr address) const
    {
        uint8 byte;
        return MemRead(address, &byte, sizeof(byte));
    }
};