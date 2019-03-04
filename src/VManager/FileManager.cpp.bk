// Copyright (C) 2019 Yu Yang
// 
// This file is part of Vesyla.
// 
// Vesyla is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Vesyla is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Vesyla.  If not, see <http://www.gnu.org/licenses/>.


#include "FileManager.hpp"
#include "VFileNames.hpp"
#include <algorithm>

FileManager::FileManager()
{
	setConfigFilename();
}

// Initializing the singleton
FileManager *FileManager::_singletonInst = new FileManager();

// Getting singleton instance
FileManager *FileManager::inst() { return _singletonInst; }

// This method decrements the tab str position.
void FileManager::decTab()
{
	_tabStr.pop_back();
}

// This method gets the type of the current file.
FileType FileManager::getCurrentFile() const
{
	return _currFileType;
}

// This method gets the ofstream pointer of the given file type.
ofstream *FileManager::getFile(FileType fileType_)
{
	return &_files[fileType_];
}

// This method gets the name of the given file type.
const string FileManager::fileName(FileType fileType_, bool includePath_)
{
	vesyla::util::GlobalVar glv;
	string path;
	CHECK(glv.gets("$OUTPUT_DIR", path));
	const string codePath = includePath_ ? path + "code/" : "";
	const string graphPath = includePath_ ? path + "graph/" + _inputFileName + "/" : "";
	const string logPath = includePath_ ? path + "log/" + _inputFileName + "/" : "";
	string name = "";

	switch (fileType_)
	{
	case ftMakeFirstRun:
		name = codePath + gMakeFirstRunFileName + _inputFileName + ".bat";
		break;
	case ftMakeRerun:
		name = codePath + gMakeRerunFileName + _inputFileName + ".bat";
		break;
	case ftManas:
		name = codePath + gManasCodeFileName + _inputFileName + ".c";
		break;
	case ftMatlab:
		name = codePath + gMatlabCodeFileName + _inputFileName + ".m";
		break;
	case ftProfiler:
		name = codePath + gProfilerFileName + _inputFileName + ".sv";
		break;
	case ftRunCmd:
		name = codePath + gRunCmdFileName + _inputFileName + ".do";
		break;
	case ftRunGui:
		name = codePath + gRunGuiFileName + _inputFileName + ".do";
		break;
	case ftTimingModel:
		name = codePath + gTimingModelFileName + _inputFileName + ".txt";
		break;
	case ftVhdlTestbench:
		name = codePath + gVhdlCodeFileName + _inputFileName + ".vhd";
		break;
	case ftVhdlPackage:
		name = codePath + gVhdlPackageFileName + _inputFileName + ".vhd";
		break;
	case ftWave:
		name = codePath + gWaveFileName + _inputFileName + ".do";
		break;

	case ftMakeGraph:
		name = graphPath + gMakeGraphFileName + _inputFileName + ".bat";
		break;
	case ftVirDot1:
		name = graphPath + gVirDot1FileName + _inputFileName + ".dot";
		break;
	case ftVirDot2:
		name = graphPath + gVirDot2FileName + _inputFileName + ".dot";
		break;
	case ftVirDot3:
		name = graphPath + gVirDot3FileName + _inputFileName + ".dot";
		break;
	case ftVirDot4:
		name = graphPath + gVirDot4FileName + _inputFileName + ".dot";
		break;
	case ftVirElabDot1:
		name = graphPath + gVirElabDot1FileName + _inputFileName + ".dot";
		break;
	case ftVirElabDot2:
		name = graphPath + gVirElabDot2FileName + _inputFileName + ".dot";
		break;
	case ftVirElabDot3:
		name = graphPath + gVirElabDot3FileName + _inputFileName + ".dot";
		break;
	case ftVirElabDot4:
		name = graphPath + gVirElabDot4FileName + _inputFileName + ".dot";
		break;

	case ftDumpDs:
		name = logPath + gDumpDsFileName + _inputFileName + ".txt";
		break;
	case ftDumpElabDs:
		name = logPath + gDumpElabDsFileName + _inputFileName + ".txt";
		break;
	case ftInstructions:
		name = logPath + gInstructionsFileName + _inputFileName + ".txt";
		break;
	case ftOptimizedSchedules:
		name = logPath + gOptimizedSchedulesFileName + _inputFileName + ".txt";
		break;
	case ftSchedules:
		name = logPath + gSchedulesFileName + _inputFileName + ".txt";
		break;
	case ftSyncInstructions:
		name = logPath + gSyncInstructionsFileName + _inputFileName + ".txt";
		break;

	case ftElaborator:
		name = gElabFileName;
		break;
	case ftParser:
		name = gParserFileName;
		break;
	case ftReport:
		name = gReportFileName;
		break;
	}

	return name;
}

// This method increments the tab str position.
void FileManager::incTab()
{
	_tabStr += "\t";
}

const string &FileManager::inputFileName()
{
	return _inputFileName;
}

void FileManager::inputFileName(const string &inputFileName_, bool createDebugDirectories_)
{
	// Find paths inside the file name and strip those paths from the file name
	size_t beginPos = inputFileName_.find_last_of("\\");
	if (beginPos == string::npos)
		beginPos = inputFileName_.find_last_of("/");

	beginPos = beginPos == string::npos ? 0 : beginPos + 1;

	_inputFileName = inputFileName_.substr(beginPos, inputFileName_.find_last_of(".") - beginPos);

	_inputFilePath = beginPos == 0 ? "" : inputFileName_.substr(0, beginPos - 1);

	if (_inputFilePath != "") // Replace all / with \ for consistency.
		std::replace(_inputFilePath.begin(), _inputFilePath.end(), '/', '\\');

	createDirectories(createDebugDirectories_);
	openAllFiles();
	resetTab();
}

// This method gets the input file path.
const string &FileManager::inputFilePath()
{
	return _inputFilePath;
}

// This method resets the tab str position.
void FileManager::resetTab()
{
	_tabStr = "";
}

// This method restores the current file back to the previous active file.
void FileManager::restoreCurrentFile()
{
	saveOrRestoreCurrentState(true);
}

// This method sets the current file, in order to be able to write to it.
void FileManager::setCurrentFile(FileType fileType_)
{
	saveOrRestoreCurrentState(false);

	_currFileType = fileType_;
	resetTab();
}

// This method writes the given string line to the current file.
void FileManager::writeLine(const string &line_)
{
	_files[_currFileType] << "\n"
												<< _tabStr << line_;
	//! Yu Yang 2017-08-01
	// Add flush to dist in order to garantee log message are kept even if program ends abnormally
	// CHANGE BEGIN
	_files[_currFileType].flush();
	// CHANGE END
}

// This method writes the given string line to the given file.
// The third argument determines if the current tab should be applied or not.
void FileManager::writeLine(const string &line_, FileType fileType_, bool useTab_)
{
	_files[fileType_] << "\n"
										<< (useTab_ ? _tabStr : "") << line_;
	//! Yu Yang 2017-08-01
	// Add flush to dist in order to garantee log message are kept even if program ends abnormally
	// CHANGE BEGIN
	_files[_currFileType].flush();
	// CHANGE END
}

// This method writes a token to the current file.
void FileManager::writeToken(const string &token_, const int width_ /*= -1*/)
{
	ofstream *currFile = &_files[_currFileType];

	if (width_ != -1)
	{
		currFile->width(width_);
		*currFile << std::left;
	}

	*currFile << token_;
	//! Yu Yang 2017-08-01
	// Add flush to dist in order to garantee log message are kept even if program ends abnormally
	// CHANGE BEGIN
	_files[_currFileType].flush();
	// CHANGE END
}

//! Yu Yang 2017-08-01
// Add close function to close all the files in the end
// CHANGE BEGIN
// This method closes all files
void FileManager::closeAllFiles()
{
	for (int fileIndex = 0; fileIndex < cFileNumber; ++fileIndex)
	{
		_files[fileIndex].close();
	}
}
// CHANGE END

void FileManager::setConfigFilename()
{
#ifdef WINDOWS
	char *pBuf = (char *)malloc(1024 * sizeof(char));
	int bytes = GetModuleFileName(NULL, pBuf, 1024);
	if (bytes >= 0)
		pBuf[bytes] = '\0';
	for (; bytes >= 0; bytes--)
	{
		if (pBuf[bytes] == '/')
		{
			pBuf[bytes] = '\0';
			break;
		}
	}
	strcat(pBuf, "/../hw_setting.conf");
	gHWSettingFileName = pBuf;
#else
	char *gHWSettingFileName = (char *)malloc(1024 * sizeof(char));
	char *pBuf = (char *)malloc(1024 * sizeof(char));
	char szTmp[32];
	sprintf(szTmp, "/proc/%d/exe", getpid());
	int bytes = readlink(szTmp, pBuf, 1024);
	if (bytes > 1023)
	{
		bytes = 1023;
	}
	if (bytes >= 0)
		pBuf[bytes] = '\0';
	for (; bytes >= 0; bytes--)
	{
		if (pBuf[bytes] == '/')
		{
			pBuf[bytes] = '\0';
			break;
		}
	}
	strcat(pBuf, "/../hw_setting.conf");
	gHWSettingFileName = pBuf;

	vesyla::util::GlobalVar glv;
	glv.puts("gHWSettingFileName", gHWSettingFileName);

#endif
}

// ---------------------------------------------------------------------
// -------------------------- Private Methods --------------------------
// ---------------------------------------------------------------------

// This method creates the needed directories
void FileManager::createDirectories(bool createDebugDirectories_)
{
	vesyla::util::GlobalVar glv;
	string path;
	string input_file_name;
	CHECK(glv.gets("$OUTPUT_DIR", path));
	string path1;
	path1 = path + "code/";
	mkdir(path1.c_str(), 0755);
	path1 = path + "graph/";
	mkdir(path1.c_str(), 0755);
	path1 = path + "log/";
	mkdir(path1.c_str(), 0755);
	CHECK(glv.gets("input_file_name", input_file_name));
	path1 = path + "graph/" + input_file_name + "/";
	mkdir(path1.c_str(), 0755);
	path1 = path + "log/" + input_file_name + "/";
	mkdir(path1.c_str(), 0755);
}

// This method closes all files and reopen all of them.
void FileManager::openAllFiles()
{
	for (int fileIndex = 0; fileIndex < cFileNumber; ++fileIndex)
	{
		_files[fileIndex].close();
		_files[fileIndex].open(fileName(FileType(fileIndex), true));
	}
}

// This method saves the current state (i.e. the current file type and the current tab level)
// in static variables, and restores them back according to the given parameter.
void FileManager::saveOrRestoreCurrentState(bool isRestore_)
{
	static FileType saveFileType;
	static string saveTabStr;

	if (!isRestore_)
	{
		saveFileType = _currFileType;
		saveTabStr = _tabStr;
	}
	else
	{
		_currFileType = saveFileType;
		_tabStr = saveTabStr;
	}
}
