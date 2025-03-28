#include "M3UParser.h"


vector<Channel> M3UParser::ParseM3UFile(const string& filePath) 
{
    vector<Channel> channels;
    ifstream file(filePath);
    string line;
    Channel currentChannel;

    // Regular expression to capture the detailed attributes
    regex detailedRegex("#EXTINF:-1.*tvg-id=\"([^\"]*)\".*tvg-name=\"([^\"]*)\".*tvg-logo=\"([^\"]*)\".*group-title=\"([^\"]*)\",(.+)$");
    smatch match;

    while (getline(file, line)) 
    {
        if (line.rfind("#EXTINF", 0) == 0) 
        {
            if (std::regex_search(line, match, detailedRegex)) 
            {
                currentChannel.id = match[1];
                currentChannel.name = match[2];
                currentChannel.logo = match[3];
                currentChannel.group = match[4];
            } 
            else
            {
                // Fallback for simpler format: extract name after the comma
                size_t commaPos = line.find(',');
                if (commaPos != std::string::npos) 
                {
                    currentChannel.name = line.substr(commaPos + 1);
                } 
                else 
                {
                    currentChannel.name = "Unknown";
                }
                // Reset other fields since they are not provided
                currentChannel.id = "";
                currentChannel.group = "";
                currentChannel.logo = "";
            }
        } 
        else if (!line.empty() && line.rfind("#", 0) != 0) 
        {
            currentChannel.url = line;
            channels.push_back(currentChannel);
        }
    }

    return channels;
}