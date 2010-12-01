#ifndef GCFS_CONTROLS_H
#define GCFS_CONTROLS_H

#include <string>
#include <vector>

class GCFS_Task;

class GCFS_Control
{
public:
						GCFS_Control(const char *sName):m_sName(sName){};
	virtual					~GCFS_Control(){};

	virtual	bool	write(GCFS_Task* pTask, const char * sValue) = 0;
	virtual	bool	read(GCFS_Task* pTask, std::string &buff) = 0;

	std::string 	trimStr(const std::string& Src, const std::string& c = " \r\n");

public:
	const char * 	m_sName;
};

class GCFS_ControlStatus : public GCFS_Control
{
public:
						GCFS_ControlStatus():GCFS_Control("status"){}

public:
	virtual	bool	write(GCFS_Task* pTask, const char * sValue);
	virtual	bool	read(GCFS_Task* pTask, std::string &buff);
	 
private:
	static const char *	statuses[];
};


class GCFS_ControlControl : public GCFS_Control
{
public:
						GCFS_ControlControl();

private:
	std::vector<const char *> 	m_vCommands;

private:
	enum eActions {
		eStart = 0,
		eStartAndWait,
		eWait,
		eAbort,
		eSuspend
	};

public:
    virtual	bool	write(GCFS_Task* pTask, const char * sValue);
    virtual	bool	read(GCFS_Task* pTask, std::string &buff);
};

#endif // GCFS_CONTROLS_H
