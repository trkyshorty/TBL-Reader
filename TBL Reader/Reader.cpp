#include <Windows.h>
#include <stdio.h>
#include <fstream>
#include "ByteBuffer.h"
#include "Table.h"
#include "Structures.h"

int main()
{
    /*
    * Define table structures
    * 
    * Retrieved from https://github.com/srmeier/KnightOnline/blob/master/Client/WarFare/GameBase.cpp
    */

    Table<__TABLE_TEXTS> s_pTbl_Texts;
    Table<__TABLE_UPC_SKILL> s_pTbl_Skill;
    Table<__TABLE_ITEM_BASIC> s_pTbl_Items_Basic;

    /*
    * Load Text Tables
    */

    if (s_pTbl_Texts.Load("texts_us.tbl"))
    {
        printf("texts_us.tbl is loaded, row size: %d\n", s_pTbl_Texts.GetDataSize());

        for (auto const& x : s_pTbl_Texts.GetData())
            printf("%d - %s\n", x.second.dwID, x.second.szText.c_str());
    }
    else
    {
        printf("Failed load: texts_us.tbl");
    }

    /*
    * Load Skill Tables
    */

    if (s_pTbl_Skill.Load("skill_magic_main_us.tbl"))
    {
        printf("skill_magic_main_us.tbl is loaded, row size: %d\n", s_pTbl_Skill.GetDataSize());

        for (auto const& x : s_pTbl_Skill.GetData())
            printf("%d - %s\n", x.second.dwID, x.second.szName.c_str());
    }
    else
    {
        printf("Failed load: skill_magic_main_us.tbl"); 
    }

    /*
    * Load Item Tables
    */

    if (s_pTbl_Items_Basic.Load("item_org_us.tbl"))
    {
        printf("item_org_us.tbl is loaded, row size: %d\n", s_pTbl_Items_Basic.GetDataSize());

        for (auto const& x : s_pTbl_Items_Basic.GetData())
            printf("%d - %s\n", x.second.dwID, x.second.szName.c_str());
    }
    else
    {
        printf("Failed load: item_org_us.tbl");
    }

    /*
    * Release Tables
    * 
    * Note: Clear tables from memory with this if you need
    * 
    */

    s_pTbl_Texts.Release();
    s_pTbl_Skill.Release();
    s_pTbl_Items_Basic.Release();
}

