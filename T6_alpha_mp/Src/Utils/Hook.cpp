#include "..\Std_Include.hpp"
#include "Hook.hpp"

#pragma section(".text")
__declspec(allocate(".text")) BYTE Utils::Hook::Detour::trampolineBuffer[200 * 20];
SIZE_T Utils::Hook::Detour::trampolineSize = 0;

namespace Utils
{
	namespace Hook
	{
		Detour::Detour()
			: place_(NULL)
			, target_(NULL)
			, originalLength_(0)
			, trampolineAddress(NULL)
		{
		}

		Detour::Detour(void* place, void* target)
			: place_(NULL)
			, target_(NULL)
			, originalLength_(0)
			, trampolineAddress(NULL)
		{
			Create(place, target);
		}

		Detour::Detour(size_t place, void* target)
			: place_(NULL)
			, target_(NULL)
			, originalLength_(0)
			, trampolineAddress(NULL)
		{
			Create(reinterpret_cast<void*>(place), target);
		}

		Detour::~Detour()
		{
			Clear();
		}

		bool Detour::Create(void* place, void* target)
		{
			if (originalLength_ != 0)
			{
				return FALSE;
			}

			place_ = place;
			target_ = target;

			size_t hookSize = WriteFarBranch(NULL, target_, FALSE, FALSE);
			memcpy(originalInstructions_, place_, hookSize);
			originalLength_ = hookSize;

			trampolineAddress = &trampolineBuffer[trampolineSize];

			for (size_t i = 0; i < hookSize / 4; ++i)
			{
				UINT32* instrPtr = (UINT32*)((UINT32)place_ + (i * 4));
				trampolineSize += CopyInstruction((UINT32*)&trampolineBuffer[trampolineSize], instrPtr);
			}

			void* afterBranchAddr = (void*)((UINT32)place_ + hookSize);
			trampolineSize += WriteFarBranch(&trampolineBuffer[trampolineSize], afterBranchAddr, FALSE, TRUE);

			WriteFarBranch(place_, target_, FALSE, FALSE);
			return TRUE;
		}

		bool Detour::Create(size_t place, void* target)
		{
			return Create(reinterpret_cast<void*>(place), target);
		}

		bool Detour::Clear()
		{
			if (place_ && originalLength_ != 0)
			{
				memcpy(place_, originalInstructions_, originalLength_);
				originalLength_ = 0;
				place_ = NULL;
				return TRUE;
			}
			return FALSE;
		}

		size_t Detour::CopyInstruction(UINT32* dest, const UINT32* src)
		{
			UINT32 instr = *src;
			switch (instr & POWERPC_OPCODE_MASK)
			{
			case POWERPC_OPCODE_B:
			case POWERPC_OPCODE_BC:
				return RelocateBranch(dest, src);
			default:
				*dest = instr;
				return 4;
			}
		}

		size_t Detour::RelocateBranch(UINT32* dest, const UINT32* src)
		{
			UINT32 instr = *src;
			UINT32 instrAddr = (UINT32)src;

			if (instr & POWERPC_BRANCH_ABSOLUTE)
			{
				*dest = instr;
				return 4;
			}

			INT32 offsetBitSize = 0;
			INT32 offsetBitBase = 0;
			UINT32 options = 0;
			BYTE crBit = 0;

			switch (instr & POWERPC_OPCODE_MASK)
			{
			case POWERPC_OPCODE_B:
				offsetBitSize = 24;
				offsetBitBase = 2;
				options = POWERPC_BRANCH_OPTIONS_ALWAYS;
				crBit = 0;
				break;
			case POWERPC_OPCODE_BC:
				offsetBitSize = 14;
				offsetBitBase = 2;
				options = (instr >> POWERPC_BIT32(10)) & MASK_N_BITS(5);
				crBit = (instr >> POWERPC_BIT32(15)) & MASK_N_BITS(5);
				break;
			}

			INT32 offset = instr & (MASK_N_BITS(offsetBitSize) << offsetBitBase);
			if (offset >> ((offsetBitSize + offsetBitBase) - 1))
			{
				offset |= ~MASK_N_BITS(offsetBitSize + offsetBitBase);
			}

			void* addr = (void*)((INT32)instrAddr + offset);
			return WriteFarBranchEx(dest, addr, (instr & POWERPC_BRANCH_LINKED) != 0, TRUE, options, crBit);
		}

		size_t Detour::WriteFarBranch(void* dest, const void* target, bool linked, bool preserveReg)
		{
			return WriteFarBranchEx(dest, target, linked, preserveReg);
		}

		size_t Detour::WriteFarBranchEx(
			void* dest,
			const void* target,
			bool linked,
			bool preserveReg,
			UINT32 options,
			BYTE crBit,
			BYTE regIndex)
		{
			UINT32 farAsm[] =
			{
				POWERPC_LIS(regIndex, POWERPC_HI((UINT32)target)),
				POWERPC_ORI(regIndex, regIndex, POWERPC_LO((UINT32)target)),
				POWERPC_MTCTR(regIndex),
				POWERPC_BCCTR(options, crBit, linked)
			};

			UINT32 farAsmPreserve[] =
			{
				POWERPC_STD(regIndex, -0x30, 1),
				POWERPC_LIS(regIndex, POWERPC_HI((UINT32)target)),
				POWERPC_ORI(regIndex, regIndex, POWERPC_LO((UINT32)target)),
				POWERPC_MTCTR(regIndex),
				POWERPC_LD(regIndex, -0x30, 1),
				POWERPC_BCCTR(options, crBit, linked)
			};

			const UINT32* asmBlock = (preserveReg)
				? farAsmPreserve
				: farAsm;
			size_t asmSize = preserveReg ? sizeof(farAsmPreserve) : sizeof(farAsm);

			if (dest)
			{
				memcpy(dest, asmBlock, asmSize);
			}
			return asmSize;
		}
	}
}