/*  This file is part of simple kernel.
 Project NanoKernel (for study purposes only)
 Copyright (C) 2013  Sirius (Vdov Nikita)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cpuinfo.h>
#include <std/membase.h>
#include <std/strings.h>
#include <std/print.h>

/* CPUID Flags. */
#define CPUID_FLAG_FPU          0x1             /* Floating Point Unit. */
#define CPUID_FLAG_VME          0x2             /* Virtual Mode Extensions. */
#define CPUID_FLAG_DE           0x4             /* Debugging Extensions. */
#define CPUID_FLAG_PSE          0x8             /* Page Size Extensions. */
#define CPUID_FLAG_TSC          0x10            /* Time Stamp Counter. */
#define CPUID_FLAG_MSR          0x20            /* Model-specific registers. */
#define CPUID_FLAG_PAE          0x40            /* Physical Address Extensions. */
#define CPUID_FLAG_MCE          0x80            /* Machine Check Exceptions. */
#define CPUID_FLAG_CXCHG8       0x100           /* Compare and exchange 8-byte. */
#define CPUID_FLAG_APIC         0x200           /* On-chip APIC. */
#define CPUID_FLAG_SEP          0x800           /* Fast System Calls. */
#define CPUID_FLAG_MTRR         0x1000          /* Memory Type Range Registers. */
#define CPUID_FLAG_PGE          0x2000          /* Page Global Enable.  */
#define CPUID_FLAG_MCA          0x4000          /* Machine Check Architecture. */
#define CPUID_FLAG_CMOV         0x8000          /* Conditional move-instruction. */
#define CPUID_FLAG_PAT          0x10000         /* Page Attribute Table. */
#define CPUID_FLAG_PSE36        0x20000         /* 36-bit Page Size Extensions. */
#define CPUID_FLAG_PSN          0x40000         /* Processor Serial Number. */
#define CPUID_FLAG_CLFL         0x80000         /* CLFLUSH - fixme? */
#define CPUID_FLAG_DTES         0x200000        /* Debug Trace and EMON Store MSRs. */
#define CPUID_FLAG_ACPI         0x400000        /* Thermal Cotrol MSR. */
#define CPUID_FLAG_MMX          0x800000        /* MMX instruction set. */
#define CPUID_FLAG_FXSR         0x1000000       /* Fast floating point save/restore. */
#define CPUID_FLAG_SSE          0x2000000       /* SSE (Streaming SIMD Extensions) */
#define CPUID_FLAG_SSE2         0x4000000       /* SSE2 (Streaming SIMD Extensions - #2) */
#define CPUID_FLAG_SS           0x8000000       /* Selfsnoop. */
#define CPUID_FLAG_HTT          0x10000000      /* Hyper-Threading Technology. */
#define CPUID_FLAG_TM1          0x20000000      /* Thermal Interrupts, Status MSRs. */
#define CPUID_FLAG_IA64         0x40000000      /* IA-64 (64-bit Intel CPU) */
#define CPUID_FLAG_PBE          0x80000000      /* Pending Break Event. */

/*
 Extended Family 20-27
 Extended Model 16-19
 Processor Type 12-13
 Family Code 8-11
 Model Number 4-7
 Stepping ID 0-3
 */
#define GET_SEPPING_ID(eax)			(byte)(eax & 0x0000000f)
#define GET_MODEL_NUMBER(eax)		(byte)((eax >> 4) & 0x0000000f)
#define GET_FAMILY_CODE(eax)		(byte)((eax >> 8) & 0x0000000f)
#define GET_PROCESSOR_TYPE(eax)		(byte)((eax >> 12) & 0x00000003)
#define GET_EXTENDED_MODEL(eax)		(byte)((eax >> 16) & 0x0000000f)
#define GET_EXTENDED_FAMILY(eax)	(byte)((eax >> 20) & 0x000000ff)

/* cpu info structure */
static cpuinfo_t cpuInfo;

/* check presence of CPUID instruction */
int32_t
k_check_cpuid();
/* CPUID exec */
void
k_get_cpu_info(uint32_t mode, uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
		uint32_t *edx);

bool NO_OPTIMIZE
k_refresh_cpu_info()
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0, i, count;
	k_memset(&cpuInfo, 0, sizeof(cpuinfo_t));

	if (!k_check_cpuid())
	{
		cpuInfo.apicPresence = false;
		cpuInfo.msrSupported = false;
		cpuInfo.cpuInfoSupported = false;
		return false;
	}

	k_get_cpu_info(0, &eax, &ebx, &ecx, &edx);
	((uint32_t *) cpuInfo.vendorID)[0] = ebx;
	((uint32_t *) cpuInfo.vendorID)[1] = edx;
	((uint32_t *) cpuInfo.vendorID)[2] = ecx;

	k_get_cpu_info(1, &eax, &ebx, &ecx, &edx);
	cpuInfo.extendedFamily = GET_EXTENDED_FAMILY(eax);
	cpuInfo.extendedModel = GET_EXTENDED_MODEL(eax);
	cpuInfo.processorType = GET_PROCESSOR_TYPE(eax);
	cpuInfo.familyCode = GET_FAMILY_CODE(eax);
	cpuInfo.modelNumber = GET_MODEL_NUMBER(eax);
	cpuInfo.steppingID = GET_SEPPING_ID(eax);

	cpuInfo.apicPresence = (edx & CPUID_FLAG_APIC) ? 1 : 0;
	cpuInfo.msrSupported = (edx & CPUID_FLAG_MSR) ? 1 : 0;

	for (i = 0x80000002, count = 0; i <= 0x80000004; i++, count += 4)
	{
		k_get_cpu_info(i, &eax, &ebx, &ecx, &edx);
		((uint32_t *) cpuInfo.brandString)[count + 0] = eax;
		((uint32_t *) cpuInfo.brandString)[count + 1] = ebx;
		((uint32_t *) cpuInfo.brandString)[count + 2] = ecx;
		((uint32_t *) cpuInfo.brandString)[count + 3] = edx;
	}

	cpuInfo.cpuInfoSupported = true;

	return true;
}

void
k_cpuinfo_print(cpuinfo_t *cpuinfo)
{
	if(!cpuinfo->cpuInfoSupported)
	{
		k_print("CPU info not supported..\n");
		return ;
	}

	k_print("\nCPU VendorID: %s\n", cpuinfo->vendorID);
	k_print("CPU Brand: %s\n", cpuinfo->brandString);

	k_print("Local APIC Presence: %s\n",
			cpuinfo->apicPresence ? "true" : "false");
	k_print("MSR Operations Supported: %s\n\n",
			cpuinfo->msrSupported ? "true" : "false");
}

cpuinfo_t *
k_get_cpuinfo()
{
	return &cpuInfo;
}

