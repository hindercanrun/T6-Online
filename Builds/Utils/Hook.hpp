#pragma once

#include <xtl.h>
#include <stdint.h>

#define MASK_N_BITS(N) ((1 << (N)) - 1)

#define POWERPC_HI(X) ((X >> 16) & 0xFFFF)
#define POWERPC_LO(X) (X & 0xFFFF)

//
// PowerPC most significant bit is addressed as bit 0 in documentation.
//
#define POWERPC_BIT32(N) (31 - N)

//
// Opcode is bits 0-5. 
// Allowing for op codes ranging from 0-63.
//
#define POWERPC_OPCODE(OP)       (OP << 26)
#define POWERPC_OPCODE_ADDI      POWERPC_OPCODE(14)
#define POWERPC_OPCODE_ADDIS     POWERPC_OPCODE(15)
#define POWERPC_OPCODE_BC        POWERPC_OPCODE(16)
#define POWERPC_OPCODE_B         POWERPC_OPCODE(18)
#define POWERPC_OPCODE_BCCTR     POWERPC_OPCODE(19)
#define POWERPC_OPCODE_ORI       POWERPC_OPCODE(24)
#define POWERPC_OPCODE_EXTENDED  POWERPC_OPCODE(31) // Use extended opcodes.
#define POWERPC_OPCODE_STW       POWERPC_OPCODE(36)
#define POWERPC_OPCODE_LWZ       POWERPC_OPCODE(32)
#define POWERPC_OPCODE_LD        POWERPC_OPCODE(58)
#define POWERPC_OPCODE_STD       POWERPC_OPCODE(62)
#define POWERPC_OPCODE_MASK      POWERPC_OPCODE(63)

#define POWERPC_EXOPCODE(OP)     (OP << 1)
#define POWERPC_EXOPCODE_BCCTR   POWERPC_EXOPCODE(528)
#define POWERPC_EXOPCODE_MTSPR   POWERPC_EXOPCODE(467)

//
// SPR field is encoded as two 5 bit bitfields.
//
#define POWERPC_SPR(SPR) (UINT32)(((SPR & 0x1F) << 5) | ((SPR >> 5) & 0x1F))

//
// Instruction helpers.
//
// rD - Destination register.
// rS - Source register.
// rA/rB - Register inputs.
// SPR - Special purpose register.
// UIMM/SIMM - Unsigned/signed immediate.
//
#define POWERPC_ADDI(rD, rA, SIMM)  (UINT32)(POWERPC_OPCODE_ADDI | (rD << POWERPC_BIT32(10)) | (rA << POWERPC_BIT32(15)) | SIMM)
#define POWERPC_ADDIS(rD, rA, SIMM) (UINT32)(POWERPC_OPCODE_ADDIS | (rD << POWERPC_BIT32(10)) | (rA << POWERPC_BIT32(15)) | SIMM)
#define POWERPC_LIS(rD, SIMM)       POWERPC_ADDIS(rD, 0, SIMM) // Mnemonic for addis %rD, 0, SIMM
#define POWERPC_LI(rD, SIMM)        POWERPC_ADDI(rD, 0, SIMM)  // Mnemonic for addi %rD, 0, SIMM
#define POWERPC_MTSPR(SPR, rS)      (UINT32)(POWERPC_OPCODE_EXTENDED | (rS << POWERPC_BIT32(10)) | (POWERPC_SPR(SPR) << POWERPC_BIT32(20)) | POWERPC_EXOPCODE_MTSPR)
#define POWERPC_MTCTR(rS)           POWERPC_MTSPR(9, rS) // Mnemonic for mtspr 9, rS
#define POWERPC_ORI(rS, rA, UIMM)   (UINT32)(POWERPC_OPCODE_ORI | (rS << POWERPC_BIT32(10)) | (rA << POWERPC_BIT32(15)) | UIMM)
#define POWERPC_BCCTR(BO, BI, LK)   (UINT32)(POWERPC_OPCODE_BCCTR | (BO << POWERPC_BIT32(10)) | (BI << POWERPC_BIT32(15)) | (LK & 1) | POWERPC_EXOPCODE_BCCTR)
#define POWERPC_STD(rS, DS, rA)     (UINT32)(POWERPC_OPCODE_STD | (rS << POWERPC_BIT32(10)) | (rA << POWERPC_BIT32(15)) | ((INT16)DS & 0xFFFF))
#define POWERPC_LD(rS, DS, rA)      (UINT32)(POWERPC_OPCODE_LD | (rS << POWERPC_BIT32(10)) | (rA << POWERPC_BIT32(15)) | ((INT16)DS & 0xFFFF))

//
// Branch related fields.
//
#define POWERPC_BRANCH_LINKED    1
#define POWERPC_BRANCH_ABSOLUTE  2

#define POWERPC_BRANCH_OPTIONS_ALWAYS (20)

#define DCBF(ptr)		__dcbf(0, ptr)
#define DCBST(ptr)		__dcbst(0, ptr)
#define SYNC()			__sync()

namespace Utils
{
	namespace Hook
	{
		class Detour
		{
		public:
			Detour();
			Detour(void* place, void* target);
			Detour(size_t place, void* target);
			~Detour();

			bool Create(void* place, void* target);
			bool Create(size_t place, void* target);
			bool Clear();

			void* GetTrampoline() const { return trampolineAddress; }

			static BYTE trampolineBuffer[200 * 20];
			static SIZE_T trampolineSize;

		private:
			void* place_;
			void* target_;
			unsigned char originalInstructions_[32];
			size_t originalLength_;
			void* trampolineAddress;

			static size_t CopyInstruction(unsigned int* dest, const unsigned int* src);
			static size_t RelocateBranch(unsigned int* dest, const unsigned int* src);

			static size_t WriteFarBranch(void* dest, const void* target, bool linked, bool preserveReg);
			static size_t WriteFarBranchEx(
				void* dest,
				const void* target,
				bool linked,
				bool preserveReg,
				unsigned int options = POWERPC_BRANCH_OPTIONS_ALWAYS,
				unsigned char crBit = 0,
				unsigned char regIndex = 11);
		};

		template <typename T>
		void SetValue(size_t place, T value)
		{
			// Okay... set it
			void* basePtr = reinterpret_cast<void*>(place);
			memcpy(basePtr, &value, sizeof(T));

			// Now, flush it to tell the CPU to apply our changes
			DCBF(basePtr);
			DCBST(basePtr);
			SYNC();
		}
	}
}