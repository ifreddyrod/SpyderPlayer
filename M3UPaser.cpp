#include "M3UParser.h"

vector<Channel> M3UParser::ParseM3UFile(const string& filePath) 
{
    vector<Channel> channels;
    ifstream file(filePath);
    
    if (!file.is_open())
     {
        cerr << "Failed to open file: " << filePath << endl;
        return channels;
    }
    
    string line;
    Channel currentChannel;
    
    // Improved regex patterns to handle various attribute formats
    // Now uses a non-greedy approach for attributes with quotes
    regex idRegex("tvg-id=\"(.*?)\"");
    regex nameRegex("tvg-name=\"(.*?)\"");
    regex logoRegex("tvg-logo=\"(.*?)\"");
    regex groupRegex("group-title=\"(.*?)\"");
    regex titleRegex("#EXTINF:-1.*?,(.*?)$");
    
    // Special regex for JSON-like structures in tvg-id
    regex jsonIdRegex("tvg-id=\"\\{'id': '(.*?)'");
    
    smatch match;

    while (getline(file, line)) 
    {
        if (line.rfind("#EXTINF", 0) == 0) 
        {
            // Reset current channel for new entry
            currentChannel = Channel();
            
            // First try to extract JSON-like id
            if (regex_search(line, match, jsonIdRegex)) 
            {
                currentChannel.id = match[1];
            }
            // Fall back to regular extraction if JSON pattern doesn't match
            else if (regex_search(line, match, idRegex)) 
            {
                currentChannel.id = match[1];
            }
            
            // Extract tvg-name
            if (regex_search(line, match, nameRegex)) 
            {
                currentChannel.name = match[1];
            }
            
            // Extract tvg-logo
            if (regex_search(line, match, logoRegex)) 
            {
                currentChannel.logo = match[1];
            }
            
            // Extract group-title
            if (regex_search(line, match, groupRegex)) 
            {
                currentChannel.group = match[1];
            }
            
            // Extract title (content after comma)
            if (regex_search(line, match, titleRegex)) 
            {
                // If name wasn't extracted from tvg-name, use the title
                if (currentChannel.name.empty()) {
                    currentChannel.name = match[1];
                }
            } 
            else 
            {
                // Fallback: try to find content after the last comma
                size_t commaPos = line.rfind(',');
                if (commaPos != string::npos && commaPos + 1 < line.length()) 
                {
                    if (currentChannel.name.empty()) 
                    {
                        currentChannel.name = line.substr(commaPos + 1);
                    }
                }
            }
            
            // Ensure channel has a name
            if (currentChannel.name.empty()) 
            {
                currentChannel.name = "Unknown";
            }
            
            // Clean up HTML entities in the logo URL
            size_t pos = 0;
            while ((pos = currentChannel.logo.find("&amp;", pos)) != string::npos) 
            {
                currentChannel.logo.replace(pos, 5, "&");
                pos += 1;
            }
        } 
        else if (!line.empty() && line.rfind("#", 0) != 0) 
        {
            // This line contains the URL
            currentChannel.url = line;
            channels.push_back(currentChannel);
        }
    }

    return channels;
}