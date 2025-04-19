#include "M3UParser.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <vector>
#include <string>

string M3UParser::DecodeHTMLEntities(const string& input) 
{
    string result = input;
    
    // Replace &amp; with &
    size_t pos = 0;
    while ((pos = result.find("&amp;", pos)) != string::npos) 
    {
        result.replace(pos, 5, "&");
        pos += 1;
    }
    
    // &quot; -> "
    pos = 0;
    while ((pos = result.find("&quot;", pos)) != string::npos) 
    {
        result.replace(pos, 6, "\"");
        pos += 1;
    }
    
    // &lt; -> <
    pos = 0;
    while ((pos = result.find("&lt;", pos)) != string::npos) 
    {
        result.replace(pos, 4, "<");
        pos += 1;
    }
    
    // &gt; -> >
    pos = 0;
    while ((pos = result.find("&gt;", pos)) != string::npos) 
    {
        result.replace(pos, 4, ">");
        pos += 1;
    }
    
    // Handle numeric HTML entities like &#039; (apostrophe)
    regex numericEntityRegex("&#(\\d+);");
    smatch match;
    string tempResult = result;
    result = "";
    
    while (regex_search(tempResult, match, numericEntityRegex)) 
    {
        result += match.prefix().str();
        int charCode = stoi(match[1]);
        result += static_cast<char>(charCode);
        tempResult = match.suffix().str();
    }
    
    result += tempResult;
    return result;
}

vector<Channel> M3UParser::ParseM3UFile(const string& filePath) 
{
    vector<Channel> channels;
    
    // Estimate channel count and reserve space
    std::ifstream fileSizeCheck(filePath, std::ios::ate);
    if (!fileSizeCheck.is_open()) 
    {
        cerr << "Failed to open file: " << filePath << endl;
        return channels;
    }
    std::streamsize fileSize = fileSizeCheck.tellg();
    fileSizeCheck.close();
    size_t estimatedChannels = std::max(size_t(10000), size_t(fileSize / 500));
    channels.reserve(estimatedChannels);
    
    // Precompile regex patterns
    static const regex idRegex("tvg-id=\"(.*?)\"");
    static const regex nameRegex("tvg-name=\"(.*?)\"");
    static const regex logoRegex("tvg-logo=\"(.*?)\"");
    static const regex groupRegex("group-title=\"(.*?)\"");
    
    // Open file for efficient reading
    FILE* filePtr = fopen(filePath.c_str(), "r");
    if (!filePtr) 
    {
        cerr << "Failed to open file: " << filePath << endl;
        return channels;
    }
    
    // Use a large buffer
    constexpr size_t BUFFER_SIZE = 8 * 1024 * 1024;  // 8MB buffer
    vector<char> buffer(BUFFER_SIZE);
    
    string currentLine;
    Channel currentChannel;
    bool inExtInf = false;
    smatch match;
    
    // Process the file in chunks
    while (!feof(filePtr))
    {
        size_t bytesRead = fread(buffer.data(), 1, BUFFER_SIZE - 1, filePtr);
        if (bytesRead == 0) break;
        
        buffer[bytesRead] = '\0';
        
        string chunk(buffer.data(), bytesRead);
        size_t lineStart = 0, lineEnd;
        
        // Process each line in the chunk
        while ((lineEnd = chunk.find('\n', lineStart)) != string::npos) 
        {
            string line = chunk.substr(lineStart, lineEnd - lineStart);
            // Remove carriage return if present
            if (!line.empty() && line.back() == '\r') 
            {
                line.pop_back();
            }
            
            // Process the line
            if (line.rfind("#EXTINF", 0) == 0) 
            {
                // If we were already in an EXTINF block and encounter a new one,
                // reset without adding the previous channel (incomplete)
                inExtInf = true;
                currentChannel = Channel();
                
                // Extract tvg-id (optional)
                if (regex_search(line, match, idRegex)) 
                {
                    currentChannel.id = DecodeHTMLEntities(match[1]);
                }
                
                // Extract tvg-name (preferred for name)
                if (regex_search(line, match, nameRegex)) 
                {
                    currentChannel.name = DecodeHTMLEntities(match[1]);
                }
                
                // Extract tvg-logo (optional)
                if (regex_search(line, match, logoRegex)) 
                {
                    currentChannel.logo = DecodeHTMLEntities(match[1]);
                }
                
                // Extract group-title (optional)
                if (regex_search(line, match, groupRegex)) 
                {
                    currentChannel.group = DecodeHTMLEntities(match[1]);
                }
                
                // Extract channel name (fallback if tvg-name not present)
                if (currentChannel.name.empty()) 
                {
                    // Manually find the last comma outside of quotes
                    bool inQuotes = false;
                    size_t lastComma = string::npos;
                    for (size_t i = 0; i < line.length(); ++i) 
                    {
                        if (line[i] == '"') 
                        {
                            inQuotes = !inQuotes;
                        }
                        else if (line[i] == ',' && !inQuotes) 
                        {
                            lastComma = i;
                        }
                    }
                    
                    if (lastComma != string::npos && lastComma + 1 < line.length()) 
                    {
                        currentChannel.name = DecodeHTMLEntities(line.substr(lastComma + 1));
                    }
                }
                
                // Ensure channel has a name
                if (currentChannel.name.empty()) 
                {
                    currentChannel.name = "Unknown";
                }
            } 
            else if (inExtInf && !line.empty() && line.rfind("#", 0) != 0) 
            {
                // This line contains the URL
                currentChannel.url = line;
                channels.push_back(std::move(currentChannel));
                inExtInf = false;
            }
            // If line is empty or starts with # (e.g., #EXTVLCOPT), skip it but keep inExtInf state
            
            lineStart = lineEnd + 1;
        }
        
        // Handle any partial line at the end of the chunk
        if (lineStart < chunk.size()) 
        {
            currentLine = chunk.substr(lineStart);
        }
        else 
        {
            currentLine.clear();
        }
    }
    
    // Handle last URL without newline
    if (inExtInf && !currentLine.empty() && currentLine.rfind("#", 0) != 0) 
    {
        currentChannel.url = currentLine;
        channels.push_back(std::move(currentChannel));
        inExtInf = false;
    }
    
    fclose(filePtr);
    return channels;
}