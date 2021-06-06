#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

struct Process {
    std::string processName;
    DWORD PID = 0;
    std::vector <std::string> TID;
};

struct TableFormat {
    int width;
    char fill;
    TableFormat() : width(30), fill(' ') {}
    template<typename T>
    TableFormat& operator<<(const T& data) {
        std::cout << data << std::setw(width) << std::setfill(fill);
        return *this;
    }
    TableFormat& operator<<(std::ostream& (*out)(std::ostream&)) {
        std::cout << out;
        return *this;
    }
};

// Extract process name and process ID
BOOL GetProcessList();
// Extract thread ID
BOOL ListProcessThreads(DWORD dwOwnerPID, Process &newP);
BOOL PrintOutProcess();

static std::vector <Process> result;

int main(void)
{
    GetProcessList();
    PrintOutProcess();
    return 0;
}


BOOL GetProcessList()
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return(FALSE);
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process, and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap); // clean the snapshot object
        return(FALSE);
    }

    do
    {
        Process newProcess;
        BOOL noError = TRUE;
        std::wstring ws(pe32.szExeFile);
        std::string str(ws.begin(), ws.end());
        newProcess.processName = str;

        // Retrieve the priority class. 
        // But I dont use print this infor
        dwPriorityClass = 0;
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        if (hProcess == NULL)
        {
            noError = FALSE;
        }
        else
        {
            dwPriorityClass = GetPriorityClass(hProcess);
            if (!dwPriorityClass)
            {
                noError = FALSE;
            }

            CloseHandle(hProcess);
        }

        newProcess.PID = pe32.th32ProcessID;

        if (ListProcessThreads(pe32.th32ProcessID, newProcess))
            result.push_back(newProcess);

    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return(TRUE);
}


BOOL ListProcessThreads(DWORD dwOwnerPID, Process &newP)
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    // Take a snapshot of all running threads  
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return(FALSE);

    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if (!Thread32First(hThreadSnap, &te32))
    {
        newP.TID.push_back("Error");
        CloseHandle(hThreadSnap);
        return(FALSE);
    }
        
    // Now walk the thread list of the system,
    // and display information about each thread
    // associated with the specified process
    do
    {
        if (te32.th32OwnerProcessID == dwOwnerPID)
        {
            newP.TID.push_back(std::to_string(te32.th32ThreadID));
        }
    } while (Thread32Next(hThreadSnap, &te32));
    CloseHandle(hThreadSnap);
    return(TRUE);
}

BOOL PrintOutProcess() {
    TableFormat out;
    int size = result.size();
    int sizeTID, i, j;
    out << "No" << "Name";
    out.width = 10;
    out << "PID" << "TID";
    std::cout << std::right << "\n";
    for (i = 0; i < size; i++) {
        out.width = 30;
        out << i + 1 << result[i].processName;
        out.width = 10;
        out << result[i].PID;
        sizeTID = result[i].TID.size();
        out.width = 0;
        for (j = 0; j < sizeTID; j++) {
            if (j == sizeTID - 1)
                out << result[i].TID[j];
            else
                out << result[i].TID[j] + ", ";
        }
        std::cout << "\n\n";
    }
    return FALSE;
}