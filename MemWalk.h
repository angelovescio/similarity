/*
 Program Name: TCWindow.hpp
 Programmer: Timothy Carpenter, 
	(c) Copyright 2003 San Diego
 Description: C++ Windows Interface class.
 Platform: Win XP Home VisualC++ 6.0, Pentium
*/
// MemWalk.h: interface for the CMemWalk class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMWALK_H__E6146549_BF42_4740_B67C_F711DCBCC313__INCLUDED_)
#define AFX_MEMWALK_H__E6146549_BF42_4740_B67C_F711DCBCC313__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>
#include <vector>
#include <stdio.h>
using namespace std;
struct VMOBJECT
{
	MEMORY_BASIC_INFORMATION	mbi;
	char			szObjType[12];
	char			szModule[MAX_PATH];
	char			szSection[IMAGE_SIZEOF_SHORT_NAME];
	BOOL			bNew;
} ;
typedef	struct VMOBJECT *LPVMOBJECT ;

class CMemWalk  
{
private:
	LPVOID m_lpWalk;
	int	nCnt;
public:
	CMemWalk();
	virtual ~CMemWalk();
	int memWalk(HANDLE hOtherProcess, vector<uint8_t> &byteStream);
	LPVMOBJECT mGetPageList(int *pCnt);
};

#endif // !defined(AFX_MEMWALK_H__E6146549_BF42_4740_B67C_F711DCBCC313__INCLUDED_)
