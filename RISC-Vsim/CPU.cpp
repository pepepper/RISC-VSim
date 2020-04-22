#include "CPU.h"
#include "Memory.h"
#include "opcode.h"
#include <iostream>
#include <cstdint>
int64_t registers[32];
double floatregisters[32];
int64_t programcounter;
#define Rtype_Dest_Reg(i) ((i>>7)&0x1f)
#define Rtype_Src1_Reg(i) ((i>>15)&0x1f)
#define Rtype_Src2_Reg(i) ((i>>20)&0x1f)
#define Itype_Dest_Reg(i) ((i>>7)&0x1f)
#define Itype_Src1_Reg(i) ((i>>15)&0x1f)
#define Itype_imm(i) (((i>>31)*0xfffffffffffff800)+(i>>20)&0x7ff)
#define Stype_Src1_Reg(i) ((i>>15)&0x1f)
#define Stype_Src2_Reg(i) ((i>>20)&0x1f)
#define Stype_imm(i) (((i>>31)*0xfffffffffffff800)+((i>>20)&0x7e0)+(i>>7)&0x1f)
#define Btype_Src1_Reg(i) ((i>>15)&0x1f)
#define Btype_Src2_Reg(i) ((i>>20)&0x1f)
#define Btype_imm(i) (((i>>31)*0xfffffffffffff000)+((i<<4)&0x400)+((i>>20)&0x7e0)+((i>>7)&0x1e))
#define Utype_Dest_Reg(i) ((i>>7)&0x1f)
#define Utype_imm(i) ((i&0xfffff000)+((i>>31)*0xffffffff00000000))
#define Jtype_Dest_Reg(i) ((i>>7)&0x1f)
#define Jtype_imm(i) (((i>>31)*0xfffffffffff00000)+(i&0xff000)+((i>>10)&0x400)+((i>>20)&0x7fe))

void fetchop(){
	registers[0]=0;
	uint64_t address;
	uint32_t operation=memory[programcounter]+(memory[programcounter+1]<<8)+(memory[programcounter+2]<<16)+(memory[programcounter+3]<<24);
	int8_t result8;
	uint8_t resultu8;
	int16_t result16;
	uint16_t resultu16;
	int32_t result32;
	uint32_t resultu32;
	switch(operation&0xfe00707f){
		//RV32I ISA START
	case MATCH_BEQ:
		if(registers[Btype_Src1_Reg(operation)]==registers[Btype_Src2_Reg(operation)])programcounter+=Btype_imm(operation);
		return;
	case MATCH_BNE:
		if(registers[Btype_Src1_Reg(operation)]!=registers[Btype_Src2_Reg(operation)])programcounter+=Btype_imm(operation);
		return;
	case MATCH_BLT:
		if(registers[Btype_Src1_Reg(operation)]<registers[Btype_Src2_Reg(operation)])programcounter+=Btype_imm(operation);
		return;
	case MATCH_BGE:
		if(registers[Btype_Src1_Reg(operation)]>registers[Btype_Src2_Reg(operation)])programcounter+=Btype_imm(operation);
		return;
	case MATCH_BLTU:
		if((uint64_t)registers[Btype_Src1_Reg(operation)]<(uint64_t)registers[Btype_Src2_Reg(operation)])programcounter+=Btype_imm(operation);
		return;
	case MATCH_BGEU:
		if((uint64_t)registers[Btype_Src1_Reg(operation)]>(uint64_t)registers[Btype_Src2_Reg(operation)])programcounter+=Btype_imm(operation);
		return;
	case MATCH_JALR:
		registers[Itype_Dest_Reg(operation)]=programcounter+4;
		programcounter=(registers[Itype_Src1_Reg(operation)]+Itype_imm(operation))&0xfffffffe;
		return;
	case MATCH_JAL:
		registers[Jtype_Dest_Reg(operation)]=programcounter+4;
		programcounter+=Jtype_imm(operation);
		return;
	case MATCH_LUI:
		registers[Utype_Dest_Reg(operation)]=Utype_imm(operation);
		break;
	case MATCH_AUIPC:
		registers[Utype_Dest_Reg(operation)]=(programcounter+=Utype_imm(operation));
		return;
	case MATCH_ADDI:
		registers[Itype_Dest_Reg(operation)]=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation);
		break;
	case MATCH_SLLI:
		registers[Itype_Dest_Reg(operation)]=(uint64_t)registers[Itype_Src1_Reg(operation)]<<(uint64_t)(Itype_imm(operation)&0x3f);
		break;
	case MATCH_SLTI:
		if((int64_t)registers[Itype_Src1_Reg(operation)]<(int64_t)Itype_imm(operation))registers[Itype_Dest_Reg(operation)]=1;
		else registers[Itype_Dest_Reg(operation)]=0;
		break;
	case MATCH_SLTIU:
		if((uint64_t)registers[Itype_Src1_Reg(operation)]<(uint64_t)Itype_imm(operation))registers[Itype_Dest_Reg(operation)]=1;
		else registers[Itype_Dest_Reg(operation)]=0;
		break;
	case MATCH_XORI:
		registers[Itype_Dest_Reg(operation)]=registers[Itype_Src1_Reg(operation)]^Itype_imm(operation);
		break;
	case MATCH_SRLI:
		registers[Itype_Dest_Reg(operation)]=(uint64_t)registers[Itype_Src1_Reg(operation)]>>(uint64_t)(Itype_imm(operation)&0x3f);
		break;
	case MATCH_SRAI:
		registers[Itype_Dest_Reg(operation)]=(int64_t)registers[Itype_Src1_Reg(operation)]>>(int64_t)(Itype_imm(operation)&0x3f);
		break;
	case MATCH_ORI:
		registers[Itype_Dest_Reg(operation)]=registers[Itype_Src1_Reg(operation)]|Itype_imm(operation);
		break;
	case MATCH_ANDI:
		registers[Itype_Dest_Reg(operation)]=registers[Itype_Src1_Reg(operation)]&Itype_imm(operation);
		break;
	case MATCH_ADD:
		registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)]+registers[Rtype_Src2_Reg(operation)];
		break;
	case MATCH_SUB:
		registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)]-registers[Rtype_Src2_Reg(operation)];
		break;
	case MATCH_SLL:
		registers[Rtype_Dest_Reg(operation)]=(uint64_t)registers[Rtype_Src1_Reg(operation)]<<(uint64_t)(registers[Rtype_Src2_Reg(operation)]&0x1f);
		break;
	case MATCH_SLT:
		if((int64_t)registers[Rtype_Src1_Reg(operation)]<(int64_t)registers[Rtype_Src2_Reg(operation)])registers[Rtype_Dest_Reg(operation)]=1;
		else registers[Rtype_Dest_Reg(operation)]=0;
		break;
	case MATCH_SLTU:
		if((uint64_t)registers[Rtype_Src1_Reg(operation)]<(uint64_t)registers[Rtype_Src2_Reg(operation)])registers[Rtype_Dest_Reg(operation)]=1;
		else registers[Rtype_Dest_Reg(operation)]=0;
		break;
	case MATCH_XOR:
		registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)]^registers[Rtype_Src2_Reg(operation)];
		break;
	case MATCH_SRL:
		registers[Rtype_Dest_Reg(operation)]=(uint64_t)registers[Rtype_Src1_Reg(operation)]>>(uint64_t)(registers[Rtype_Src2_Reg(operation)]&0x1f);
		break;
	case MATCH_SRA:
		registers[Rtype_Dest_Reg(operation)]=(int64_t)registers[Rtype_Src1_Reg(operation)]>>(int64_t)(registers[Rtype_Src2_Reg(operation)]&0x1f);
		break;
	case MATCH_OR:
		registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)]|registers[Rtype_Src2_Reg(operation)];
		break;
	case MATCH_AND:
		registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)]&registers[Rtype_Src2_Reg(operation)];
		break;
	case MATCH_LB://TODO load x0 address must raise exception
		address=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation);
		result8=memory[address];
		registers[Itype_Dest_Reg(operation)]=(int64_t)result8;
		break;
	case MATCH_LH:
		address=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation);
		result16=memory[address]+(memory[address+1]<<8);
		registers[Itype_Dest_Reg(operation)]=(int64_t)result16;
		break;
	case MATCH_LW:
		address=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation);
		result32=memory[address]+(memory[address+1]<<8)+(memory[address+2]<<16)+(memory[address+3]<<24);
		registers[Itype_Dest_Reg(operation)]=(int64_t)result32;
		break;
	case MATCH_LBU:
		address=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation);
		resultu8=memory[address];
		registers[Itype_Dest_Reg(operation)]=(int64_t)resultu8;
		break;
	case MATCH_LHU:
		address=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation);
		resultu16=memory[address]+(memory[address+1]<<8);
		registers[Itype_Dest_Reg(operation)]=(int64_t)resultu16;
		break;
	case MATCH_SB:
		address=registers[Stype_Src1_Reg(operation)]+Stype_imm(operation);
		memory[address]=registers[Stype_Src2_Reg(operation)]&0xff;
		break;
	case MATCH_SH:
		address=registers[Stype_Src1_Reg(operation)]+Stype_imm(operation);
		memory[address]=registers[Stype_Src2_Reg(operation)]&0xff;
		memory[address+1]=(registers[Stype_Src2_Reg(operation)]>>8)&0xff;
		break;
	case MATCH_SW:
		address=registers[Stype_Src1_Reg(operation)]+Stype_imm(operation);
		memory[address]=registers[Stype_Src2_Reg(operation)]&0xff;
		memory[address+1]=(registers[Stype_Src2_Reg(operation)]>>8)&0xff;
		memory[address+2]=(registers[Stype_Src2_Reg(operation)]>>16)&0xff;
		memory[address+3]=(registers[Stype_Src2_Reg(operation)]>>24)&0xff;
		break;
	case MATCH_FENCE:
	case MATCH_FENCE_I:
		break;
		//RV32I ISA END
		//RV64I ISA START
	case MATCH_ADDIW:
		result32=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation)&0xffffffff;
		registers[Itype_Dest_Reg(operation)]=(int64_t)result32;
		break;
	case MATCH_SLLIW:
		resultu32=registers[Itype_Src1_Reg(operation)]<<(Itype_imm(operation)&0x1f);
		registers[Itype_Dest_Reg(operation)]=(uint64_t)resultu32;
		break;
	case MATCH_SRLIW:
		resultu32=registers[Itype_Src1_Reg(operation)]>>(Itype_imm(operation)&0x1f);
		registers[Itype_Dest_Reg(operation)]=(uint64_t)resultu32;
		break;
	case MATCH_SRAIW:
		result32=registers[Itype_Src1_Reg(operation)]>>(Itype_imm(operation)&0x1f);
		registers[Itype_Dest_Reg(operation)]=(uint64_t)result32;
		break;
	case MATCH_ADDW:
		result32=(int32_t)registers[Rtype_Src1_Reg(operation)]+(int32_t)registers[Rtype_Src2_Reg(operation)];
		registers[Rtype_Dest_Reg(operation)]=(uint64_t)result32;
		break;
	case MATCH_SUBW:
		result32=(int32_t)registers[Rtype_Src1_Reg(operation)]-(int32_t)registers[Rtype_Src2_Reg(operation)];
		registers[Rtype_Dest_Reg(operation)]=(uint64_t)result32;
		break;
	case MATCH_SLLW:
		result32=(uint32_t)registers[Rtype_Src1_Reg(operation)]<<(uint32_t)registers[Rtype_Src2_Reg(operation)];
		registers[Rtype_Dest_Reg(operation)]=(uint64_t)result32;
		break;
	case MATCH_SRLW:
		result32=(uint32_t)registers[Rtype_Src1_Reg(operation)]>>(uint32_t)registers[Rtype_Src2_Reg(operation)];
		registers[Rtype_Dest_Reg(operation)]=(uint64_t)result32;
		break;
	case MATCH_SRAW:
		result32=(int32_t)registers[Rtype_Src1_Reg(operation)]>>(int32_t)registers[Rtype_Src2_Reg(operation)];
		registers[Rtype_Dest_Reg(operation)]=(uint64_t)result32;
		break;
	case MATCH_LD:
		address=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation);
		registers[Itype_Dest_Reg(operation)]=(uint64_t)memory[address]+((uint64_t)memory[address+1]<<8)+((uint64_t)memory[address+2]<<16)+((uint64_t)memory[address+3]<<24)
			+((uint64_t)memory[address+4]<<32)+((uint64_t)memory[address+5]<<40)+((uint64_t)memory[address+6]<<48)+((uint64_t)memory[address+7]<<56);
		break;
	case MATCH_LWU:
		address=registers[Itype_Src1_Reg(operation)]+Itype_imm(operation);
		resultu8=memory[address];
		registers[Itype_Dest_Reg(operation)]=(int64_t)resultu8;
		break;
	case MATCH_SD:
		address=registers[Stype_Src1_Reg(operation)]+Stype_imm(operation);
		memory[address]=registers[Stype_Src2_Reg(operation)]&0xff;
		memory[address+1]=(registers[Stype_Src2_Reg(operation)]>>8)&0xff;
		memory[address+2]=(registers[Stype_Src2_Reg(operation)]>>16)&0xff;
		memory[address+3]=(registers[Stype_Src2_Reg(operation)]>>24)&0xff;
		memory[address+4]=(registers[Stype_Src2_Reg(operation)]>>32)&0xff;
		memory[address+5]=(registers[Stype_Src2_Reg(operation)]>>40)&0xff;
		memory[address+6]=(registers[Stype_Src2_Reg(operation)]>>48)&0xff;
		memory[address+7]=(registers[Stype_Src2_Reg(operation)]>>56)&0xff;
		break;
		//RV64I ISA END
		//RV64M ISA START
	case MATCH_MUL:
		registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)]*registers[Rtype_Src2_Reg(operation)];
		break;
	case MATCH_MULH:
		registers[Rtype_Dest_Reg(operation)]=(registers[Rtype_Src1_Reg(operation)]*registers[Rtype_Src2_Reg(operation)])>>64;
		break;
	case MATCH_MULHU:
		registers[Rtype_Dest_Reg(operation)]=((uint64_t)registers[Rtype_Src1_Reg(operation)]*(uint64_t)registers[Rtype_Src2_Reg(operation)])>>64;
		break;
	case MATCH_MULHSU:
		registers[Rtype_Dest_Reg(operation)]=(registers[Rtype_Src1_Reg(operation)]*(uint64_t)registers[Rtype_Src2_Reg(operation)])>>64;
		break;
	case MATCH_MULW:
		result32=(int32_t)registers[Rtype_Src1_Reg(operation)]*(int32_t)registers[Rtype_Src2_Reg(operation)];
		registers[Rtype_Dest_Reg(operation)]=(int64_t)result32;
		break;
	case MATCH_DIV:
		if(registers[Rtype_Src2_Reg(operation)])registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)]/registers[Rtype_Src2_Reg(operation)];
		else registers[Rtype_Dest_Reg(operation)]=-1;
		break;
	case MATCH_DIVU:
		if(registers[Rtype_Src2_Reg(operation)])registers[Rtype_Dest_Reg(operation)]=(uint64_t)registers[Rtype_Src1_Reg(operation)]/(uint64_t)registers[Rtype_Src2_Reg(operation)];
		else registers[Rtype_Dest_Reg(operation)]=-1;
		break;
	case MATCH_REM:
		if(registers[Rtype_Src2_Reg(operation)])registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)]%registers[Rtype_Src2_Reg(operation)];
		else registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)];
		break;
	case MATCH_REMU:
		if(registers[Rtype_Src2_Reg(operation)])registers[Rtype_Dest_Reg(operation)]=(uint64_t)registers[Rtype_Src1_Reg(operation)]%(uint64_t)registers[Rtype_Src2_Reg(operation)];
		else registers[Rtype_Dest_Reg(operation)]=registers[Rtype_Src1_Reg(operation)];
		break;
	case MATCH_DIVW:
		if(registers[Rtype_Src2_Reg(operation)])result32=(int32_t)registers[Rtype_Src1_Reg(operation)]/(int32_t)registers[Rtype_Src2_Reg(operation)];
		else result32=-1;
		registers[Rtype_Dest_Reg(operation)]=result32;
		break;
	case MATCH_DIVUW:
		if(registers[Rtype_Src2_Reg(operation)])resultu32=(uint32_t)registers[Rtype_Src1_Reg(operation)]/(uint32_t)registers[Rtype_Src2_Reg(operation)];
		else resultu32=0xffffffff;
		registers[Rtype_Dest_Reg(operation)]=resultu32;
		break;
	case MATCH_REMW:
		if(registers[Rtype_Src2_Reg(operation)])result32=(int32_t)registers[Rtype_Src1_Reg(operation)]%(int32_t)registers[Rtype_Src2_Reg(operation)];
		else result32=registers[Rtype_Src1_Reg(operation)];
		registers[Rtype_Dest_Reg(operation)]=result32;
		break;
	case MATCH_REMUW:
		if(registers[Rtype_Src2_Reg(operation)])resultu32=(uint32_t)registers[Rtype_Src1_Reg(operation)]%(uint32_t)registers[Rtype_Src2_Reg(operation)];
		else resultu32=registers[Rtype_Src1_Reg(operation)];
		registers[Rtype_Dest_Reg(operation)]=resultu32;
		break;
		//RV64M ISA START
		//SYSTEM ISA START
		/*
	case MATCH_ECALL:
		//TODO
		break;
	case MATCH_EBREAK:
		//TODO
		break;
	case MATCH_URET:
	case MATCH_SRET:
	case MATCH_MRET:
	case MATCH_DRET:
	case MATCH_SFENCE_VMA:
	case MATCH_WFI:
	case MATCH_CSRRW:
	case MATCH_CSRRS:
	case MATCH_CSRRC:
	case MATCH_CSRRWI:
	case MATCH_CSRRSI:
	case MATCH_CSRRCI:
	case MATCH_HFENCE_VVMA:
	case MATCH_HFENCE_GVMA:
		//SYSTEM ISA END
		*/
	default:
		std::cerr<<"opcode"<<(memory[programcounter]&0xfe00707f)<<"are not implemented.";
	}
	programcounter+=4;
}