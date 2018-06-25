
#include <iostream>
#include <dirent.h>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

using namespace std;

void printProgBar(int percent);

string getReturn(const string retType);

string getSuper(const string text);

vector<string> getAllFiles(const string dir);

string replaceAll(const std::string &str, const std::string &from, const std::string &to);

bool makePath(const std::string &path);

int main(int argc, char *argv[]) {
    if (argc == 1)
        return 1;
    string inputPath;
    string outputPath;
    if (argc > 1) {
        inputPath = string(argv[1]);
        if (inputPath.find_last_of('/') == inputPath.length() - 1) {
            inputPath.replace(inputPath.length() - 1, 1, "");
        }
        outputPath = inputPath + "_out";
    }
    if (argc > 2) {
        outputPath = string(argv[2]);
    }
    cout << "input: " << inputPath << endl;
    cout << "output: " << outputPath << endl;
    try {
        vector<string> fileNames = getAllFiles(inputPath);
        size_t N = fileNames.size();
        size_t prog = 0;
        for (string fileName : fileNames) {
            ++prog;
            float percent = (prog / (float) N) * 100.0;
            printProgBar(percent);
            ifstream file(fileName);
            stringstream buffer;
            if (file) {
                buffer << file.rdbuf();
                file.close();
            }

            string line;
            int lineNum = 0;
            bool isRegex = false;
            bool isString = false;
            bool isFunction = false;
            bool isHaveAttr = false;
            int backSlash = 0;
            int bracket = 0;
            string className = "";
            string retType = "";
            string superText = "";
            vector<string> lines;
            size_t funcPos;
            size_t offset;
            size_t maxOffset;

            while (getline(buffer, line, '\n')) {
                ++lineNum;
                size_t offset = 0;
                size_t maxOffset = line.length();
                if (!isFunction) {
                    size_t funcPos = line.find("function");
                    if (funcPos != string::npos) {
                        isFunction = true;
                        isRegex = false;
                        isString = false;
                        backSlash = 0;
                        superText = "";
                        if (line.find(";\r") != string::npos) {
                            lines.push_back(line);
                            isFunction = false;
                            continue;
                        }
                        if (line.find("#") != string::npos) {
                            line = replaceAll(line, "#", "");
                        }
                        if (line.find("function goto(") != string::npos) {
                            line = replaceAll(line, "function goto(", "function Goto(");
                        }
                        if (line.find("override function") != string::npos) {
                            line = replaceAll(line, "override function", "override protected function");
                            funcPos += 10;
                            maxOffset += 10;
                        }
                        isHaveAttr = false;
                        while (offset < funcPos) {
                            if (line[offset] != ' ') {
                                isHaveAttr = true;
                                break;
                            }
                            ++offset;
                        }
                        if (!isHaveAttr && line.find(className, 0) == string::npos) {
                            line = replaceAll(line, "function", "protected function");
                            funcPos += 10;
                            maxOffset += 10;
                        }
                        size_t index1 = line.find(")", funcPos);
                        size_t index2 = string::npos;
                        size_t beg = string::npos;
                        size_t end = string::npos;
                        if (index1 != string::npos) {
                            index2 = line.find(":", index1);
                            lines.push_back(line.substr(0, index1 + 1));
                        }
                        if (index2 != string::npos) {
                            beg = index2 + 1;
                            end = maxOffset;
                            while (beg < maxOffset && line[beg] == ' ') beg++;
                            end = beg;
                            while (end < maxOffset && line[end] != ' ' && line[end] != '\r') end++;
                            retType = line.substr(beg, end - beg);
                            if (retType != "") {
                                lines.push_back(getReturn(retType));
                            }
                        }
                    } else {
                        size_t classPos = line.find("class");
                        if (classPos != string::npos) {
                            size_t beg = classPos + 5;
                            size_t end = beg;
                            while (beg < maxOffset && line[beg] == ' ') beg++;
                            end = beg;
                            while (end < maxOffset && line[end] != ' ' && line[end] != '\r') end++;
                            className = line.substr(beg, end - beg);
                        }
                        if (line.find("import", 0) != string::npos &&
                            line.find("§") != string::npos)
                            continue;
                        if (line.find(" //", 0) != string::npos)
                            line = replaceAll(line, " //", " /");
                        lines.push_back(line);
                    }
                } else {
                    if (superText == "") superText = getSuper(line);
                    while (offset < maxOffset) {
                        if (line[offset] == '/' && backSlash % 2 == 0 && line[offset + 1] != ' ' && !isString) {
                            isRegex = !isRegex;
                        }
                        if (line[offset] == '\"' && backSlash % 2 == 0 && !isRegex) {
                            isString = !isString;
                        }
                        if (line[offset] == '{' && !isString) {
                            bracket++;
                        }
                        if (line[offset] == '}' && !isString) {
                            bracket--;
                            if (bracket == 0) {
                                isFunction = false;
                                lines.push_back(superText);
                            }
                        }
                        if (line[offset] == '\\') ++backSlash;
                        else backSlash = 0;
                        offset++;
                    }
                }
            }
            string outFileName = replaceAll(fileName, inputPath, outputPath);
            std::string outDir = outFileName.substr(0, outFileName.find_last_of('/'));
            if (makePath(outDir)) {
//            cout << "mkdir: " << outDir << endl;
//                if (fileName.find("AccumulativeLoginAnalyer", 0) != string::npos) {
//                    int i = 1;
//                    for(vector<string>::iterator it = lines.begin(); it != lines.end(); it++, i++){
//                        cout << *it << endl;
//                    }
//                    cout << "length: " << i << endl;
//                }
                ofstream myfile(outFileName, std::ofstream::out);
                for (auto line : lines) {
                    myfile << line;
                }
                myfile.close();
            }
        }
    }
    catch (exception &e) {
        cout << e.what() << '\n';
    }

    return 0;
}


bool isDirExist(const std::string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

bool makePath(const std::string &path) {
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
    if (ret == 0)
        return true;

    switch (errno) {
        case ENOENT: {
            int pos = (int) path.find_last_of('/');
            if (pos == std::string::npos)
                return false;
            if (!makePath(path.substr(0, pos)))
                return false;
        }
            // now, try to create again
            return 0 == mkdir(path.c_str(), mode);

        case EEXIST:
            // done!
            return isDirExist(path);
        case EACCES:
            cout << "permission is denied" << endl;
            return false;
        case ELOOP:
            return false;
        case EMLINK:
            return false;
        case ENAMETOOLONG:
            return false;
        case ENOSPC:
            return false;
        case ENOTDIR:
            return false;
        case EROFS:
            return false;
        default:
            return false;
    }
}

string replaceAll(const std::string &str, const std::string &from, const std::string &to) {
    string ret = string(str);
    if (from.empty())
        return ret;
    size_t start_pos = 0;
    while ((start_pos = ret.find(from, start_pos)) != std::string::npos) {
        ret.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return ret;
}

string getSuper(const string text) {
    string superText = "";
    size_t superPos = text.find("super(");
    if (text[superPos + 6] == ')')
        return " { super(); }\n";
    size_t superEndPos = superPos + 6;
    if (superPos != string::npos) {
        superEndPos = text.find(")", superEndPos);
        if (superEndPos != string::npos) {
            size_t offset = superPos;
            size_t maxOffset = superEndPos;
            superText = " { super(";
            int count = 1;
            for (; offset < maxOffset; offset++) {
                if (text[offset] == ',') ++count;
            }
            for (int i = 1; i < count; i++) {
                superText += "null,";
            }
            superText += "null); }\n";
        }
    }
    return superText;
}

string getReturn(const string retType) {
    if (retType == "void") return " : void { }\n";
    if (retType == "int") return " : int { return 0; }\n";
    if (retType == "Number") return " : Number { return 0; }\n";
    if (retType == "Boolean") return " : Boolean { return false; }\n";
    return " : " + retType + " { return null; }\n";
}

vector<string> getAllFiles(const string dir_name) {
    vector<string> files;
    DIR *dirp = opendir(dir_name.c_str());
    dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }
        string path = dir_name + "/" + dp->d_name;
        if (dp->d_type == DT_DIR) {
            vector<string> subFiles = getAllFiles(path);
            files.insert(files.end(), subFiles.begin(), subFiles.end());
        } else if (dp->d_type == DT_REG) {
            files.push_back(path);
        }
    }
    (void) closedir(dirp);
    return files;
}

void printProgBar(int percent) {
    string bar;

    for (int i = 0; i < 50; i++) {
        if (i < (percent / 2)) {
            bar.replace(i, 1, "=");
        } else if (i == (percent / 2)) {
            bar.replace(i, 1, ">");
        } else {
            bar.replace(i, 1, " ");
        }
    }

    cout << "\r" "processing [" << bar << "] ";
    cout.width(3);
    cout << percent << "%     " << std::flush;
}