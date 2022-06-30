#include <cstdlib>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "windows.h"
#include "Objbase.h"
#include "Shlobj.h"

using namespace std;

TCHAR select_directory_path[MAX_PATH];

// TODO: A lot of error detection and handling is missing.
/*
class COM {
public:
  COM() {
  if (CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) != S_OK)
    throw runtime_error("Failed to initialize COM.");
  }
  ~COM() {
    CoUninitialize();
  }
};
*/

class file_search {
public:
  WIN32_FIND_DATA find_data;
  HANDLE handle;

  file_search(string search_path) : find_data({}) {
    handle = FindFirstFile(search_path.c_str(), &find_data);
  }

  ~file_search() {
    if (handle != nullptr) FindClose(handle);
  }

  bool find_next() {
    return FindNextFile(handle, &find_data);
  }

  inline const char* found_filename() { return find_data.cFileName; };
};

string select_directory() {
  BROWSEINFO browseinfo {};
  browseinfo.pszDisplayName = select_directory_path;
  browseinfo.lpszTitle = "Please select directory containing the bin files.";
  browseinfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
  PIDLIST_ABSOLUTE idlist = SHBrowseForFolder(&browseinfo);
  if (idlist == nullptr) {
    return "";
  }
  else {
    if (!SHGetPathFromIDList(idlist, select_directory_path)) {
      CoTaskMemFree(idlist);
      throw runtime_error("SHGetPathFromIDList failed.");
    };
    CoTaskMemFree(idlist);
    return string(select_directory_path);
  }
}

vector<string> find_bin_files(string directory) {
  vector<string> result;

  file_search fs(directory + "\\*.bin");
  if (GetLastError() != ERROR_FILE_NOT_FOUND) {
    result.emplace_back(fs.found_filename());
    while (fs.find_next()) {
      result.emplace_back(fs.found_filename());
    }
  }

  return result;
}

string generate_cuesheet(vector<string> files) {
  stringstream ss;

  if (files.size() > 0) {
    ss << "FILE \"" << files.at(0) << "\" BINARY\n";
    ss << "  TRACK 01 MODE2/2352\n";
    ss << "    INDEX 01 00:00:00\n";
    for(size_t track = 1; track < files.size(); ++track) {
      ss << "FILE \"" << files.at(track) << "\" BINARY\n";
      ss << "  TRACK ";
      size_t track2 = track + 1;
      if (track2 < 10) ss << '0';
      ss << track2 << " AUDIO\n";
      ss << "   INDEX 00 00:00:00\n";
      ss << "   INDEX 01 00:02:00\n";
    };
  }

  return ss.str();
}

string get_cuesheet_filename(string directory, vector<string> files) {
  TCHAR filename[MAX_PATH];
  filename[0] = 0;
  OPENFILENAME save_dialog {};
  save_dialog.lStructSize = sizeof(save_dialog);
  save_dialog.lpstrFilter = "Cuesheet files (*.cue)\0*.cue\0All files\0*";
  save_dialog.lpstrDefExt = "cue";
  save_dialog.lpstrFile = filename;
  save_dialog.lpstrInitialDir = directory.c_str();
  save_dialog.nMaxFile = MAX_PATH;
  save_dialog.Flags = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
  save_dialog.FlagsEx = OFN_EX_NOPLACESBAR;

  if (GetSaveFileName(&save_dialog)) {
    return string(filename);
  }
  else {
    return "";
  }
}

bool file_exists(string filename) {
  file_search fs(filename);
  auto error_code = GetLastError();

  if (error_code == ERROR_FILE_NOT_FOUND) return false;
  if (error_code == ERROR_NO_MORE_FILES) return true;
  throw error_code;
}

int main(int argc, const char* argv[]) {
  string path;
  string filename;

  if (argc > 1) {
    int i = 1;
    while (i < argc) {
      string a = argv[i];
      if (a == "--path" && i + 1 < argc) {
        path = argv[++i];
        cout << "Path = " << path << endl;
      }
      if (a == "--output" && i + 1 < argc) {
        filename = argv[++i];
        cout << "Cue filename = " << filename << endl;
      }
      i++;
    }
  }

  try {
    //COM com;

    string dir;
    if (path.empty()) {
      dir = select_directory();
    } else {
      dir = path;
    }
    if (!dir.empty()) {
      auto files = find_bin_files(dir);
      if (files.empty()) throw runtime_error("No bin files found in the selected directory.");
      auto cuesheet = generate_cuesheet(files);
      if (filename.empty()) {
        filename = get_cuesheet_filename(dir, files);
      } else {
        if (path.back() == '\\' or path.back() == '/') {
            filename = path + filename;
        } else {
            filename = path + "\\" + filename;
        }
      }
      if (!filename.empty()) {
        ofstream file(filename.c_str(), ios::out);
        file << cuesheet.c_str();
      }
    }
  }
  catch (const exception& e) {
    MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
    return EXIT_FAILURE;
  }
  catch (DWORD error_code) {
    LPSTR buffer = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
      error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
    MessageBox(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
    LocalFree(buffer);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
