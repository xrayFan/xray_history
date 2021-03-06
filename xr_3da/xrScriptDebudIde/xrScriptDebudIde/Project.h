// Project.h: interface for the CProject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROJECT_H__07580CB8_BA8B_47B6_9813_78E51B8C971C__INCLUDED_)
#define AFX_PROJECT_H__07580CB8_BA8B_47B6_9813_78E51B8C971C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ProjectFile.h"

class CWorkspaceWnd;

typedef CTypedPtrArray<CPtrArray, CProjectFile*> CProjectFileArray;

class CProject  
{
public:
	void RemoveFile(CProjectFile* pPF);
	BOOL Close();
	BOOL New();
	BOOL New(CString);
	BOOL HasBreakPoint(const char* szFile, int nLine);
	CProject();
	virtual ~CProject();

	BOOL PositionBreakPoints();
	BOOL CheckBeforeBuild();

	void SetModifiedFlag(BOOL bModified) { m_bModified = bModified; };
	void SaveModified();
	BOOL SaveAs();
	BOOL Save();
	BOOL Save(CString strPathName);
	BOOL Save(CArchive& ar);
	BOOL Load();
	BOOL Load(CArchive& ar);
	BOOL Load(CString);

	void RemoveProjectFiles();
	void AddFile(CString strPathName, BOOL bFindExisting=TRUE);
	void AddFile(CProjectFile* pPF);
	int NofFiles() { return m_files.GetSize(); };
	void AddFiles();
	CProjectFile* GetProjectFile(CString strPathName);
	void RedrawFilesTree();
	void NewProject();
	CString GetProjectDir();
	void SetPathName(CString strPathName) { m_strPathName=strPathName; };
	CString GetPathName() { return m_strPathName; };
	CString GetName();
	CString GetNameExt();

protected:
	BOOL m_bModified;

	CProjectFileArray	m_files;
	CString m_strPathName;

};

#endif // !defined(AFX_PROJECT_H__07580CB8_BA8B_47B6_9813_78E51B8C971C__INCLUDED_)
