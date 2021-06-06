#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <iomanip>
#include <unistd.h>


struct Process {
    std::string processName;
    std::string PID;
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

static std::vector <Process> result;
static std::vector <std::string> listProcess;

bool PrintOutProcess();
void getInformationProcess();
// Return an array string after spliting
std::vector<std::string> splitStringByWhiteSpace(std::string str);
// Execute command line in terminal
std::string exec(const char* cmd);
// Check a file exists or not
inline bool exists_file(const std::string& name);

int main() {
    std::string process = exec("ls /proc/ | awk '!a[$0]++' |grep -o '[0-9]\\+'| sort -n | awk '{print}' ORS=' '");
    listProcess = splitStringByWhiteSpace(process);
    getInformationProcess();
    PrintOutProcess();
    return 0;
}

inline bool exists_file(const std::string& name) {
    return (access(name.c_str(), F_OK) != -1);
}

std::vector<std::string> splitStringByWhiteSpace(std::string str)
{
    std::vector <std::string> newArrString;
    // Used to split string around spaces.
    std::istringstream ss(str);
    std::string word;
    while (ss >> word)
    {
        newArrString.push_back(word);
    }
    return newArrString;
}

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    }
    catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

void getInformationProcess() {
    int n = listProcess.size(), i;
    std::string cmd, listTID;
    for (i = 0; i < n; i++) {
        Process newP;
        newP.PID = listProcess[i];
        std::string file = "/proc/" + listProcess[i] + "/status";
        // Check if a file exists or not
        if (!exists_file(file))
            continue;
        // Get the name of process
        cmd = "cat /proc/" + listProcess[i] + "/status | grep \"Name\" | awk '{split($0,a,\" \"); print a[2]}'| tr '\\n' ' '";
        newP.processName = exec((cmd).c_str());
        if (newP.processName.empty())
            continue;
        // Get thread id
        cmd = "ls /proc/" + listProcess[i] + "/task | awk '{gsub(\" \",\", \"); print}' | sort -n | awk '{print}' ORS=' '";
        listTID = exec((cmd).c_str());
        newP.TID = splitStringByWhiteSpace(listTID);
        result.push_back(newP);
    }
}


bool PrintOutProcess() {
    TableFormat out;
    int size = result.size();
    int sizeTID, i, j;
    out.width = 30;
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
        std::cout << "\n";
    }
    return true;
}