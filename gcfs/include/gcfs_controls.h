#ifndef GCFS_CONTROLS_H
#define GCFS_CONTROLS_H

#include <string>
#include <vector>

#include "gcfs_filesystem.h"

class GCFS_Task;

class GCFS_Control : public GCFS_FileSystem
{
public:
                        GCFS_Control(GCFS_Task * pTask);
   virtual	           ~GCFS_Control(){};
};

class GCFS_ControlStatus : public GCFS_Control
{
public:
                        GCFS_ControlStatus(GCFS_Task * pTask);

public:
   virtual ssize_t      read(std::string sBuffer, off_t uiOffset, size_t uiSize);
   virtual ssize_t      write(const char * sBuffer, off_t uiOffset, size_t uiSize);

private:
   static const char *  statuses[];
};


class GCFS_ControlControl : public GCFS_Control
{
public:
                        GCFS_ControlControl(GCFS_Task * pTask);

public:
   bool                 executeCommand(GCFS_Task* pTask, int iCommandIndex);

private:
   std::vector<const char *> m_vCommands;

private:
   enum eActions {
      eStart = 0,
      eStartAndWait,
      eWait,
      eAbort,
      eSuspend
   };

public:
   virtual ssize_t      read(std::string sBuffer, off_t uiOffset, size_t uiSize);
   virtual ssize_t      write(const char * sBuffer, off_t uiOffset, size_t uiSize);
};


#endif // GCFS_CONTROLS_H
