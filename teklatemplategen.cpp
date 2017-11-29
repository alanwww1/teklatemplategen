
#include <iostream>
#include <string>
#include <stdio.h>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <sstream>
#include <algorithm>
#include <vector>
#include <array>


using namespace std;

std::map<std::string, std::string> mapSettings;  // This will hold the current settings for a text or value or other RPT entry.
std::vector<std::string> vecSettingsOrder;   //We will hold a sort order for the settings stored in the mapSettings map

std::map<std::string, std::string> xmlEscapes = { {"&amp;", "&"} , {"&apos;", "\'"}, {"&quot;", "\""}, {"&gt;",">"}, {"&lt;", "<"}, {"&#10;", "\n"} };

struct CXLSRowData
{
  std::string strParameter;
  std::string strRowText;
};

std::vector<CXLSRowData> vecHeaderData;
std::vector<CXLSRowData> vecRowData;
std::vector<CXLSRowData> vecFooterData;

size_t ilastRowEnd = 0;
size_t iHeaderLineCountXML =0;
size_t iRowLineCountXML =0;
size_t iFooterLineCountXML =0;

void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos)
  {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

std::string UnEscapeXMLstr(std::string strXMLText)
{
    for (const auto& strEscape : xmlEscapes)
    {
      ReplaceStringInPlace(strXMLText, strEscape.first, strEscape.second);   
    }
    return strXMLText;
};


bool FileExist(std::string filename)
{
  FILE* pfileToTest = fopen (filename.c_str(),"rb");
  if (pfileToTest == NULL)
    return false;
  fclose(pfileToTest);
  return true;
};

std::string ReadFileToStr(std::string strFileName)
{
  std::string strEmpty;
  if (!FileExist(strFileName))
    return strEmpty;

  FILE * file;
  std::string strRead;
  file = fopen(strFileName.c_str(), "rb");
  if (!file)
    printf("FileUtils: ReadFileToStr: unable to read file: %s", strFileName.c_str());

  fseek(file, 0, SEEK_END);
  int64_t fileLength = ftell(file);
  fseek(file, 0, SEEK_SET);

  strRead.resize(static_cast<size_t> (fileLength));

  unsigned int readBytes =  fread(&strRead[0], 1, fileLength, file);
  fclose(file);

  if (readBytes != fileLength)
  {
    printf("FileUtils: actual read data differs from file size, for string file: %s",strFileName.c_str());
  }
  return strRead;
};

bool WriteFileFromStr(const std::string &filename, std::string const &strToWrite)
{

  FILE * pFile = fopen (filename.c_str(),"wb");
  if (pFile == NULL)
  {
    printf("FileUtils: WriteFileFromStr failed for file: %s\n", filename.c_str());
    return false;
  }
  fprintf(pFile, "%s", strToWrite.c_str());
  fclose(pFile);

  return true;
};

std::string escapeStringToCPlus(std::string &strIn, bool skipFirstLast = false)
{
  std::string strOut;
  for (string::iterator it = strIn.begin();it != strIn.end(); it++)
  {
    bool bFirstOrLastChar = (it == strIn.begin()) || (it == (strIn.end()-1));
    if ((bFirstOrLastChar && skipFirstLast) || (*it != '\"') )
    {
      if (*it != '\r')
        strOut += *it;
    }
    else
      strOut += "\\\"";
  }
  return strOut;
}

void AddLineToMap(std::string strKey, std::string strValue)
{
  if (mapSettings.find(strKey) == mapSettings.end())
    vecSettingsOrder.push_back(strKey);

  mapSettings[strKey] = strValue;
}


void SetRPTEntryDefaults(std::string strSettingEntry, std::string strEntryType, int iPosX, int iPosY)
{
  mapSettings.clear();
  vecSettingsOrder.clear();

  std::string strPosX = to_string(iPosX);
  std::string strPosY = to_string(iPosY);

  if (strEntryType == "textEntry")
  {
    AddLineToMap("name", "\"Text_" + strPosY + "_" + strPosX + "\"");
    AddLineToMap("x1", strPosX);
    AddLineToMap("y1", strPosY);
    AddLineToMap("x2", strPosX);
    AddLineToMap("y2", strPosY);
    AddLineToMap("string", "\"" + strSettingEntry + "\"");
    AddLineToMap("fontname","\"Courier New\"");
    AddLineToMap("fontcolor","153");
    AddLineToMap("fonttype","2");
    AddLineToMap("fontsize","1");
    AddLineToMap("fontratio","1");
    AddLineToMap("fontslant","0");
    AddLineToMap("fontstyle","0");
    AddLineToMap("angle","0");
    AddLineToMap("justify","LEFT");
    AddLineToMap("pen","-1");
  }

  if (strEntryType == "valueEntry")
  {
    AddLineToMap("name", "\"Value_" + strPosY + "_" + strPosX + "\"");
    AddLineToMap("location", "(" + strPosX + ", " + strPosY + ")");
    AddLineToMap("formula", "\"\"");
    AddLineToMap("datatype", "STRING");
    AddLineToMap("class", "\"\"");
    AddLineToMap("cacheable", "TRUE");
    AddLineToMap("justify","LEFT");
    AddLineToMap("visibility", "TRUE");
    AddLineToMap("angle","0");
    AddLineToMap("length","10");
    AddLineToMap("sortdirection","NONE");
    AddLineToMap("fontname","\"Courier New\"");
    AddLineToMap("fontcolor","153");
    AddLineToMap("fonttype","2");
    AddLineToMap("fontsize","1");
    AddLineToMap("fontratio","1");
    AddLineToMap("fontstyle","0");
    AddLineToMap("fontslant","0");
    AddLineToMap("pen","0");
    AddLineToMap("oncombine","NONE");
  }

  if (strEntryType == "PageHeader")
  {
    AddLineToMap("name", "\"Header\"");
  }

  if (strEntryType == "PageFooter")
  {
    AddLineToMap("name", "\"PageFooter\"");
  }
  
  if (strEntryType == "rowEntry")
  {
    AddLineToMap("name", "\"Row_" + strPosY + "_" + strPosX + "\"");
//    AddLineToMap("height","1");
    AddLineToMap("visibility","TRUE");
    AddLineToMap("usecolumns","TRUE");
    AddLineToMap("rule","\"\"");
    AddLineToMap("contenttype","\"PART\"");
    AddLineToMap("sorttype","COMBINE");
  }
}


void ParseSettingsValeEntryFromXML(std::string strSettingEntry, int iPosX, int iPosY)
{
  std::string strPosX = to_string(iPosX);
  std::string strPosY = to_string(iPosY);
  size_t iCursor = 0, iCursorNext = 0;

  while ((iCursor = strSettingEntry.find("$",iCursor)) != std::string::npos)
  {
    iCursorNext = strSettingEntry.find("$",iCursor+1);

    if (iCursorNext == std::string::npos)
      throw std::invalid_argument("No terminating \"$\" character in paramteric string.\nString: %s" + strSettingEntry);

    size_t iPosColon = strSettingEntry.find(":", iCursor);
    if (iCursorNext < iPosColon)
      throw std::invalid_argument("No \":\" character separating value and key texts in paramteric string.\nString: %s" + strSettingEntry);
    
    
    std::string strKey = strSettingEntry.substr(iCursor+1, iPosColon - iCursor-1);
    std::string strValue = strSettingEntry.substr(iPosColon+1, iCursorNext - iPosColon -1);

    strValue = UnEscapeXMLstr(strValue);
    strValue = escapeStringToCPlus(strValue,true);
    
    AddLineToMap(strKey, strValue);

    iCursor = iCursorNext +1;
  };
}

size_t AddEntry(std::string &strOutputFile, std::string strEntryType, int iPosX, int iPosY)
{
  std::vector<std::string>::iterator itVecSettings;
 
  std::string strPosX = to_string(iPosX);
  std::string strPosY = to_string(iPosY);
  std::string strLeadingSpace;

  if (strEntryType == "textEntry")
  {
    strOutputFile += "        text _tmp_" + strPosY + "_" + strPosX + "\n";
    strOutputFile += "        {\n";
    strLeadingSpace = "            ";
  }

  if (strEntryType == "valueEntry")
  {
    strOutputFile +=   "        valuefield _tmp_" + strPosY + "_" + strPosX + "\n";
    strOutputFile +=   "        {\n";
    strLeadingSpace = "            ";
  }
  
  if (strEntryType == "PageHeader")
  {
    strOutputFile +=   "    header _tmp_" + strPosY + "_" + strPosX + "\n";
    strOutputFile +=   "    {\n";
    strLeadingSpace = "        ";
  }
  
  if (strEntryType == "PageFooter")
  {
    strOutputFile +=   "    footer _tmp_" + strPosY + "_" + strPosX + "\n";
    strOutputFile +=   "    {\n";
    strLeadingSpace = "        ";
  }
  
    if (strEntryType == "rowEntry")
  {
    strOutputFile +=   "    row _tmp_" + strPosY + "_" + strPosX + "\n";
    strOutputFile +=   "    {\n";
    strLeadingSpace = "        ";
  }

   
  for (itVecSettings = vecSettingsOrder.begin(); itVecSettings != vecSettingsOrder.end(); itVecSettings++)
  {
    std::string strKey = *itVecSettings;
    std::string strValue = mapSettings[strKey];

    strOutputFile += strLeadingSpace + strKey + " = " + strValue + ";\n";
  };
  
  if (strEntryType == "textEntry" || strEntryType == "valueEntry")
    strOutputFile +=   "        };\n\n";    
  
  if (strEntryType == "valueEntry")
    return stoi(mapSettings["length"]);

  return 0;
};

void ParseRowData(std::string strInputFile,  std::string strRowSearchString, std::vector<CXLSRowData> &vecCurrentVector)
{
  size_t iLastParamStart = 0;
  while ((iLastParamStart = strInputFile.find(strRowSearchString,iLastParamStart + 1)) != std::string::npos)
  {

    CXLSRowData CurrentXMLSRowData;
  
    CurrentXMLSRowData.strParameter = strInputFile.substr(iLastParamStart, strInputFile.find("%", iLastParamStart + 1)-iLastParamStart);

    size_t iEndOfRow = strInputFile.find("</Row>",iLastParamStart);

    if (iEndOfRow == std::string::npos)
      throw std::invalid_argument("No terminating Row entry after RowDescription");

    iEndOfRow+= 6;

    size_t iStartOfRow = 0;
  
    if (strRowSearchString != "%HEAD" || vecCurrentVector.size() != 0) // If we are haveing the first Header row, let's use have the complete test before that as well.
    {
      iStartOfRow = strInputFile.rfind("<Row",iLastParamStart);

      if (iStartOfRow == std::string::npos)
        throw std::invalid_argument("No starting Row entry before RowDescription");
  
      iStartOfRow = strInputFile.rfind(">", iStartOfRow);
      iStartOfRow = strInputFile.find_first_of("< \t", iStartOfRow);
    }

    CurrentXMLSRowData.strRowText = strInputFile.substr(iStartOfRow, iEndOfRow-iStartOfRow);

    size_t iPosCellStart = CurrentXMLSRowData.strRowText.rfind("<Cell", iLastParamStart - iStartOfRow);
    iPosCellStart = CurrentXMLSRowData.strRowText.rfind ("</Cell>", iPosCellStart)+7;
    iPosCellStart =CurrentXMLSRowData.strRowText.find_first_of(" \t",iPosCellStart);

    size_t iPosCellEnd = CurrentXMLSRowData.strRowText.find("</Cell>" , iLastParamStart - iStartOfRow)+7;
    iPosCellEnd = CurrentXMLSRowData.strRowText.find_first_not_of("\n\r",iPosCellEnd);
    
    CurrentXMLSRowData.strRowText.replace(iPosCellStart, iPosCellEnd-iPosCellStart, "");
    
    if (strRowSearchString == "%HEAD" || strRowSearchString == "%ROW")
      ilastRowEnd = iEndOfRow;

    if ((strRowSearchString == "%FOOT") && (strInputFile.find(strRowSearchString,iLastParamStart + 1) == std::string::npos)) // we have the last footer line
      CurrentXMLSRowData.strRowText += strInputFile.substr(iEndOfRow);  //Lets link the rest of the text to its end.
    
    vecCurrentVector.push_back(CurrentXMLSRowData);

    if (strRowSearchString =="%HEAD")
      iHeaderLineCountXML += count(CurrentXMLSRowData.strRowText.begin(), CurrentXMLSRowData.strRowText.end(), '\n');
    if (strRowSearchString =="%ROW")
      iRowLineCountXML += count(CurrentXMLSRowData.strRowText.begin(), CurrentXMLSRowData.strRowText.end(), '\n');
    if (strRowSearchString =="%FOOT")
      iFooterLineCountXML += count(CurrentXMLSRowData.strRowText.begin(), CurrentXMLSRowData.strRowText.end(), '\n');
    
// printf("%s\n%s\n%s\n", strRowSearchString.c_str(), CurrentXMLSRowData.strParameter.c_str(), CurrentXMLSRowData.strRowText.c_str());
  }

  if (strRowSearchString == "%FOOT" && vecCurrentVector.size() ==0 ) // we did not have a footer line
  {
    std::string strRestofText = strInputFile.substr(ilastRowEnd);
    vecCurrentVector.push_back({"",strRestofText});  //Lets add the rest of the text.
    iFooterLineCountXML += count(strRestofText.begin(), strRestofText.end(), '\n');
//printf("%s\n%s\n%s\n", strRowSearchString.c_str(), "", strRestofText.c_str());
  }
}

std::string CreateRTPSection(std::string &strInputFile, size_t iStartLine)
{
  stringstream sstrString (strInputFile);
  std::string strOutputFile;

  std::string strLine;
  int iLineCounter =0;
  std::string strError;

//  size_t iLineCount = std::count(strInputFile.begin(), strInputFile.end(), '\n');
  size_t iLineCount = iStartLine;

  while (std::getline(sstrString, strLine))
  {
    if (*strLine.end() == '\n')
      strLine = strLine.substr(0, strLine.length()-1);

    strLine = escapeStringToCPlus(strLine);
//    std::string strLineNumber = std::to_string(iLineCounter);
    
    if (strInputFile.find("%HEAD") == 0 || strInputFile.find("%ROW") == 0 || strInputFile.find("%FOOT") == 0)
    {
        // we have a special entry with header, footer, row information
        std::string sType;
        if (strInputFile.find("%HEAD") == 0)
            sType = "PageHeader";
        if (strInputFile.find("%FOOT") == 0)
            sType = "PageFooter";
        if (strInputFile.find("%ROW") == 0)
            sType = "rowEntry";

        SetRPTEntryDefaults(strInputFile, sType, 0, iLineCount - iLineCounter);
        ParseSettingsValeEntryFromXML(strInputFile, 0, iLineCount - iLineCounter);
        AddEntry(strOutputFile, sType, 0, iLineCount - iLineCounter );
        return strOutputFile;
    }
    

    if (strLine.find("%") != std::string::npos) // we have a parametric text here
    {
      size_t iLastpos = 0, iFirstpos =0, iNextPos =0, iPosXCounter = 0;
      iFirstpos = strLine.find("%", iNextPos);

      do
      {
        if (iNextPos != 0)   // we a re reading not the first entry starting with '%' character
          iLastpos = iNextPos+1;

        iNextPos = strLine.find("%", iFirstpos+1);
        if (iNextPos == std::string::npos)
        {
          strError = "No second \"%\" character to close the parameter\nLine: %s" + strLine;
          throw std::invalid_argument( strError);
        }

        std::string strPreEntry = strLine.substr(iLastpos, iFirstpos-iLastpos);
        std::string strEntry = strLine.substr(iFirstpos, iNextPos-iFirstpos+1);

        
        SetRPTEntryDefaults(strPreEntry, "textEntry", iPosXCounter, iLineCount - iLineCounter);
        AddEntry(strOutputFile, "textEntry", iPosXCounter, iLineCount - iLineCounter );

        iPosXCounter += strPreEntry.length() - std::count (strPreEntry.begin(), strPreEntry.end(), '\\');

        SetRPTEntryDefaults(strEntry, "valueEntry", iPosXCounter, iLineCount - iLineCounter);
        ParseSettingsValeEntryFromXML(strEntry, iPosXCounter, iLineCount - iLineCounter);
        iPosXCounter += AddEntry(strOutputFile, "valueEntry", iPosXCounter, iLineCount - iLineCounter );

        iFirstpos = strLine.find("%", iNextPos+1);
      }
      while (iFirstpos != std::string::npos);

      std::string strPostEntry = strLine.substr(iNextPos+1);
      SetRPTEntryDefaults(strPostEntry, "textEntry", iPosXCounter, iLineCount - iLineCounter);
      AddEntry(strOutputFile, "textEntry", iPosXCounter, iLineCount - iLineCounter );
    }
    else  // we have a line without any variables
    {
      SetRPTEntryDefaults(strLine, "textEntry", 0, iLineCount - iLineCounter);
      AddEntry(strOutputFile, "textEntry", 0, iLineCount - iLineCounter );
    }

    iLineCounter++;
//DEBUG
    //WriteFileFromStr("/home/atti/Dokumentumok/teklatemplategen/output.txt", strOutputFile);
  }
  return strOutputFile;
}

std::string HandleCellComments(std::string strInput)
{
// Move formulas from Comment sections to the cell text places 
  size_t iCommStart = 0;
  while ((iCommStart = strInput.find("<Comment")) != std::string::npos)
  {
    size_t iCommEnd = strInput.find("</Comment>", iCommStart) +10;
    iCommEnd = strInput.find_first_not_of("\r\n", iCommEnd);
        
    size_t iFontStart = strInput.rfind("<Font", iCommEnd);

    size_t iCommTextStart = strInput.find(">", iFontStart +1)+1;
    size_t iCommTextEnd = strInput.find("<", iCommTextStart)-1;
    std::string strCommentText = strInput.substr(iCommTextStart, iCommTextEnd-iCommTextStart +1);
    
    if (strCommentText.find("&#10;") == 0)
        strCommentText = strCommentText.substr(5, strCommentText.length() -5);

    size_t iCellStart = strInput.rfind("<Cell",iCommStart);
    size_t iDataStart = strInput.find("<Data", iCellStart);
    size_t iCellTextStart = strInput.find(">",iDataStart +1) +1;
//    size_t iCellTextEnd = strInput.find("<",iCellTextStart)-1;
    
    strInput.replace(iCommStart, iCommEnd- iCommStart, "");
    strInput.insert(iCellTextStart, strCommentText);
  }
//  printf("%s\n", strInput.c_str());
  return strInput;
}

int main(int argc, char* argv[])
{
  std::string strInputFile, strOutputFile;
  strInputFile=ReadFileToStr("/home/atti/Dokumentumok/teklatemplategen/input.xml");
  if (strInputFile.empty())
  {
    printf ("input file not found or empty");
    return 1;
  }
  
  std::string strXMLHeader = "<?xml version=\"1.0\"?>";
  size_t iPosXMLMainHeader = strInputFile.find(strXMLHeader);

  if (iPosXMLMainHeader == std::string::npos)
    throw std::invalid_argument("Input file is not a valid xml file");
  
  strInputFile.replace(iPosXMLMainHeader, strXMLHeader.size(), "<?xml version=\"1.0\" encoding=\"windows-1252\"?>");  //This is the encoding windows Excel expects for an inputfile

//  size_t XMLLineCount = count(strInputFile.begin(), strInputFile.end(), '\n');
  strInputFile = HandleCellComments(strInputFile);
  
  ParseRowData(strInputFile, "%HEAD", vecHeaderData);
  ParseRowData(strInputFile, "%ROW", vecRowData);
  ParseRowData(strInputFile, "%FOOT", vecFooterData);
     
  std::vector<CXLSRowData>::iterator itVec;
  size_t iMaxHeight = iHeaderLineCountXML + iFooterLineCountXML + iRowLineCountXML+2;
  std::string sMaxHeight = to_string(iMaxHeight);

  strOutputFile += ""
  "template\n"
  "{\n"
  "    name = \"template_Excel_XML\";\n"
  "    type = TEXTUAL;\n"
  "    width = 300;\n";
  strOutputFile += "    maxheight = " + sMaxHeight + ";\n";
  strOutputFile += ""
  "    columns = (1, 1);\n"
  "    gap = 1;\n"
  "    fillpolicy = EVEN;\n"
  "    filldirection = HORIZONTAL;\n"
  "    fillstartfrom = TOPLEFT;\n"
  "    margins = (0, 0, 0, 0);\n"
  "    gridxspacing = 1;\n"
  "    gridyspacing = 1;\n"
  "    version = 3.5;\n"
  "    created = \"29.10.2004 12:02\";\n"
  "    modified = \"27.11.2017 15:17\";\n"
  "    notes = \"\";\n\n";
  
  
  size_t iPosY = iHeaderLineCountXML+1;
  for (itVec = vecHeaderData.begin(); itVec != vecHeaderData.end(); itVec ++)
  {
    if (itVec == vecHeaderData.begin())
    {
      strOutputFile += CreateRTPSection(itVec->strParameter, iMaxHeight);
      strOutputFile += "        height = " + to_string(iPosY+1) + ";\n";
    } 
    strOutputFile += CreateRTPSection(itVec->strRowText, iPosY);      
    iPosY = iPosY -std::count(itVec->strRowText.begin(), itVec->strRowText.end(), '\n')-1;
  }
  
  strOutputFile += "    };\n\n";
  
  iPosY = iRowLineCountXML;
  
  for (itVec = vecRowData.begin(); itVec != vecRowData.end(); itVec ++)
  {
    strOutputFile += CreateRTPSection(itVec->strParameter, iMaxHeight-iHeaderLineCountXML -iPosY );
    strOutputFile += "        height = " + to_string(std::count(itVec->strRowText.begin(), itVec->strRowText.end(), '\n')+1) + ";\n";

    strOutputFile += CreateRTPSection(itVec->strRowText, iPosY);      
    iPosY = iPosY -std::count(itVec->strRowText.begin(), itVec->strRowText.end(), '\n');
    strOutputFile += "    };\n";
  }
  
//  strOutputFile = CreateRTPSection(strInputFile);

  iPosY = iFooterLineCountXML-1;
  for (itVec = vecFooterData.begin(); itVec != vecFooterData.end(); itVec ++)
  {
    if (itVec == vecFooterData.begin())
    {
      if (itVec->strParameter.empty())
        itVec->strParameter = "%FOOT%";
      strOutputFile += CreateRTPSection(itVec->strParameter, iMaxHeight - iHeaderLineCountXML -iRowLineCountXML- iPosY);
      strOutputFile += "        height = " + to_string(iPosY+1) + ";\n";
    } 
    strOutputFile += CreateRTPSection(itVec->strRowText, iPosY);
    iPosY = iPosY -std::count(itVec->strRowText.begin(), itVec->strRowText.end(), '\n');
  }
  
  strOutputFile += "    };\n";
  strOutputFile += "};\n";
  
  if (!WriteFileFromStr("/home/atti/Dokumentumok/teklatemplategen/output.txt", strOutputFile))
  {
    printf ("Output file write error");
    return 1;
  }

//  printf ("%s", strOutputFile.c_str());
}
