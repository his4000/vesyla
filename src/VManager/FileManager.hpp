
#ifndef _FileManager_H_
#define _FileManager_H_

#include "util/Common.hpp"
#include "FileType.hpp"
#include <string>
#include <fstream>

using std::ofstream;
using std::string;

#define DecTab() FileManager::inst()->decTab()
#define GetCurrentFile() FileManager::inst()->getCurrentFile()
#define GetFile(fileType_) FileManager::inst()->getFile(fileType_)
#define GetFileName(fileType_, includePath_) FileManager::inst()->fileName(fileType_, includePath_)
#define IncTab() FileManager::inst()->incTab()
#define InputFileName() FileManager::inst()->inputFileName()
#define InputFilePath() FileManager::inst()->inputFilePath()
#define ResetTab() FileManager::inst()->resetTab()
#define RestoreCurrentFile() FileManager::inst()->restoreCurrentFile()
#define SetCurrentFile(fileType_) FileManager::inst()->setCurrentFile(fileType_)
#define WriteLine(line_) FileManager::inst()->writeLine(line_)
#define WriteLineToF(line_, fileType_, useTab_) FileManager::inst()->writeLine(line_, fileType_, useTab_)
#define WriteToken(token_, width_) FileManager::inst()->writeToken(token_, width_)

//! Yu Yang 2017-08-01
// Add close function to close all the files in the end
// CHANGE BEGIN
#define CLOSE_ALL_FILES FileManager::inst()->closeAllFiles()
// CHANGE END

class FileManager
{
public:
	FileManager();
	//! Singleton implementation
	static FileManager *inst();

	//! Yu Yang 2017-08-01
	// Remove inline decorator to avoid compile error in gcc
	// CHANGE BEGIN
	void decTab();
	FileType getCurrentFile() const;
	ofstream *getFile(FileType fileType_);
	const string fileName(FileType fileType_, bool includePath_);
	void incTab();
	const string &inputFileName();
	void inputFileName(const string &inputFileName_, bool createDebugDirectories_);
	const string &inputFilePath();
	void resetTab();
	void restoreCurrentFile();
	void setCurrentFile(FileType fileType_);
	void writeLine(const string &line_);
	void writeLine(const string &line_, FileType fileType_, bool useTab_);
	void writeToken(const string &token_, const int width_ = -1);

	// CHANGE END

	//! Yu Yang 2017-08-01
	// Add close function to close all the files in the end
	// CHANGE BEGIN
	void closeAllFiles();
	// CHANGE END

	void setConfigFilename();

private:
	string _inputFileName;
	string _inputFilePath;
	FileType _currFileType;
	ofstream _files[cFileNumber];
	string _tabStr;

	void createDirectories(bool createDebugDirectories_);
	void openAllFiles();
	void saveOrRestoreCurrentState(bool isRestore_);

private: //! Singleton implementation
	static FileManager *_singletonInst;
	FileManager(FileManager const &);
	void operator=(FileManager const &);
};

#endif // _FileManager_H_
