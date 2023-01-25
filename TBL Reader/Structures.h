#pragma once
#include <string>

typedef struct  __TABLE_TEXTS
{
	uint32_t			dwID;
	std::string			szText;
} TABLE_TEXTS;

typedef struct __TABLE_UPC_SKILL
{
	uint32_t			dwID;
	std::string			szEngName;
	std::string			szName;
	std::string			szDesc;
	int32_t				iSelfAnimID1;
	int32_t				iSelfAnimID2;

	int32_t				idwTargetAnimID;
	int32_t				iSelfFX1;
	int32_t				iSelfPart1;
	int32_t				iSelfFX2;
	int32_t				iSelfPart2;
	int32_t				iFlyingFX;
	int32_t				iTargetFX;

	int32_t				iTargetPart;
	int32_t				iTarget;
	int32_t				iNeedLevel;
	int32_t				iNeedSkill;
	int32_t				iExhaustMSP;
	int32_t				iExhaustHP;

	int16_t				sUnknown1;			//19

	int32_t				iCastTime;
	uint32_t			dwNeedItem;
	int32_t				iReCastTime;
	int32_t				iCooldown;

	float				fUnknown1;			//24

	uint8_t				byPercentSuccess;

	float				fUnknown2;			//26
	int32_t				iUnknown3;			//27

	uint32_t			dw1stTableType;
	uint32_t			dw2ndTableType;
	int32_t				iValidDist;

	int32_t				iUnknown4;			//31
	uint32_t			dwUnknown5;			//32
	int32_t				iUnknown6;			//33

	uint32_t			iBaseId;

	uint8_t				byUnknown8;			//35
	uint8_t				byUnknown9;			//36

} TABLE_UPC_SKILL;


typedef struct __TABLE_ITEM_BASIC
{
	uint32_t			dwID;
	uint8_t 			byExtIndex;
	std::string			szName;
	std::string			szDesc;

	uint32_t			dwIDK0;
	uint8_t				byIDK1;

	uint32_t			dwIDResrc;
	uint32_t			dwIDIcon;
	uint32_t			dwSoundID0;
	uint32_t			dwSoundID1;

	uint8_t				byKind;
	uint8_t				byIsRobeType;
	uint8_t				byAttachPoint;
	uint8_t				byNeedRace;
	uint8_t				byNeedClass;

	int16_t				siDamage;
	int16_t				siAttackInterval;
	int16_t				siAttackRange;
	int16_t				siWeight;
	int16_t				siMaxDurability;
	int					iPriceRepair;
	int					iPriceSale;
	int16_t				siDefense;
	uint8_t				byContable;

	uint32_t			dwEffectID1;
	uint32_t			dwEffectID2;

	int8_t				byReqLevelMin; 
	int8_t				byReqLevelMax;


	uint8_t				byNeedRank;
	uint8_t				byNeedTitle;
	uint8_t				byNeedStrength;
	uint8_t				byNeedStamina;
	uint8_t				byNeedDexterity;
	uint8_t				byNeedInteli;
	uint8_t				byNeedMagicAttack;

	uint8_t				bySellGroup;

	uint8_t				byItemGrade;

	int32_t				iUnknown1;
	int16_t				siBound;

	uint32_t			dwUnknown2;
	uint8_t				byUnknown3;
} TABLE_ITEM_BASIC;
