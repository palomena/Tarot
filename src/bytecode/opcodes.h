#ifndef TAROT_OPCODES_H
#define TAROT_OPCODES_H

/* Only available to tarot source files */
#ifdef TAROT_SOURCE

#include "defines.h"

enum tarot_opcode {
	OP_NoOperation,
	OP_Halt,
	OP_Debug,
	OP_Assert,
	OP_Try,
	OP_RaiseException,
	OP_Goto,
	OP_GotoIfFalse,
	OP_CallForeignFunction,
	OP_CallFunction,
	OP_Return,
	OP_ReturnValue,
	/* MARK: Memory */
	OP_PushRegion,
	OP_PopRegion,
	OP_LoadValue,
	OP_StoreValue,
	OP_CopyValue,
	OP_PopValue,
	OP_LoadArgument,
	OP_LoadVariablePointer,
	OP_LoadListIndex,
	OP_LoadDictIndex,
	OP_Track,
	OP_UnTrack,
	/* MARK: Logical */
	OP_PushTrue,
	OP_PushFalse,
	OP_LogicalAnd,
	OP_LogicalOr,
	OP_LogicalXor,
	OP_LogicalEquality,
	OP_LogicalNot,
	/* MARK: Integer */
	OP_PushInteger,
	OP_CopyInteger,
	OP_FreeInteger,
	OP_StoreInteger,
	OP_CastToInteger,
	OP_IntegerAbs,
	OP_IntegerNeg,
	OP_IntegerAddition,
	OP_IntegerSubtraction,
	OP_IntegerMultiplication,
	OP_IntegerDivision,
	OP_IntegerModulo,
	OP_IntegerPower,
	OP_IntegerLessThan,
	OP_IntegerLessEqual,
	OP_IntegerGreaterThan,
	OP_IntegerGreaterEqual,
	OP_IntegerEquality,
	/* MARK: Float */
	OP_PushFloat,
	OP_CastToFloat,
	OP_FloatAbs,
	OP_FloatNeg,
	OP_FloatAddition,
	OP_FloatSubtraction,
	OP_FloatMultiplication,
	OP_FloatDivision,
	OP_FloatModulo,
	OP_FloatPower,
	OP_FloatLessThan,
	OP_FloatLessEqual,
	OP_FloatGreaterThan,
	OP_FloatGreaterEqual,
	OP_FloatEquality,
	/* MARK: Rational */
	OP_PushRational,
	OP_CopyRational,
	OP_StoreRational,
	OP_FreeRational,
	OP_CastToRational,
	OP_RationalAbs,
	OP_RationalNeg,
	OP_RationalAddition,
	OP_RationalSubtraction,
	OP_RationalMultiplication,
	OP_RationalDivision,
	OP_RationalModulo,
	OP_RationalPower,
	OP_RationalLessThan,
	OP_RationalLessEqual,
	OP_RationalGreaterThan,
	OP_RationalGreaterEqual,
	OP_RationalEquality,
	/* MARK: String */
	OP_PushString,
	OP_CopyString,
	OP_StoreString,
	OP_FreeString,
	OP_CastToString,
	OP_StringEquality,
	OP_StringContains,
	OP_StringConcat,
	OP_StringLength,
	/* MARK: List */
	OP_PushList,
	OP_ListIndex,
	OP_FreeList,
	OP_ListLength,
	OP_ListAppend,
	OP_PushDict,
	OP_DictIndex,
	OP_FreeDict,
	/* MARK: I/O */
	OP_PrintBoolean,
	OP_PrintInteger,
	OP_PrintFloat,
	OP_PrintRational,
	OP_PrintString,
	OP_PrintList,
	OP_PrintDict,
	OP_StoreList,
	OP_CopyList,
	OP_NewLine,
	OP_Input
};

/**
 * Returns the name of the opcode as a constant string.
 */
extern const char* opcode_string(enum tarot_opcode opcode);

#endif /* TAROT_SOURCE */

#endif /* TAROT_OPCODES_H */
