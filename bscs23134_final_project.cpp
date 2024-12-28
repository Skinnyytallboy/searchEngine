#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <thread>
#include <algorithm>

// define namespace and alias
namespace fs = std::filesystem;

// Signs that will/can be used while searching
#define addSign '+'
#define subSign '-'
#define spcSign ' '

// Folder name which contains all the files
//#define mainDir "review_text"
#define mainDir "tempFolder"


// Enum and struct to help in searchEngine class
enum searchType { defaultSearch, addSearch, subSearch, sentenceSearch, invalidSearch };

// Class to store the word and its position in the document
class wordInDocument{
    private:
        std::string documentName;
        std::vector<int> positions;
    public:
		wordInDocument(const std::string& docName) : documentName(docName) {}
        wordInDocument(const std::string& docName, int pos) : documentName(docName) { positions.push_back(pos); }
    	void addPosition(int pos) { positions.push_back(pos); }
    	std::string getDocumentName() const { return documentName; }
    	const std::vector<int>& getPositions() const { return positions; }
    	int getFrequency() const { return positions.size(); }
        bool appearsInDocument(const std::string& docName) const { return documentName == docName; }
        bool appearsInPosition(int pos) const { return std::find(positions.begin(), positions.end(), pos) != positions.end(); }
        bool operator==(const wordInDocument& other) const { return documentName == other.documentName; }
        ~wordInDocument() = default;
};

class searchEngine {
    private:
        std::unordered_map<std::string, std::vector<wordInDocument>> filesMap;

        void queryType(const std::string& query, searchType& type) {
            type = invalidSearch;
            std::string tempQuery = query;
            std::transform(tempQuery.begin(), tempQuery.end(), tempQuery.begin(), ::tolower);
            int plusCount = std::count(tempQuery.begin(), tempQuery.end(), addSign);
            int minusCount = std::count(tempQuery.begin(), tempQuery.end(), subSign);
            if (tempQuery.find(spcSign) == std::string::npos) type = defaultSearch;
            else if (plusCount > 0 && minusCount == 0) type = addSearch;
            else if (minusCount > 0 && plusCount == 0) type = subSearch;
            else if (tempQuery.find(' ') != std::string::npos) type = sentenceSearch;
            else type = invalidSearch;
        }    

        std::vector<std::pair<std::string, int>> searchDefault(const std::string& query) {
            std::vector<std::pair<std::string, int>> results;
            std::istringstream iss(query);
            std::string word;
            while (iss >> word) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                if (filesMap.find(word) != filesMap.end()) {
                    for (const auto& doc : filesMap[word]) {
                        bool found = false;
                        for (auto& result : results) {
                            if (result.first == doc.getDocumentName()) {
                                result.second += doc.getFrequency();  // Increment the frequency if already exists
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            results.push_back({doc.getDocumentName(), doc.getFrequency()});
                        }
                    }
                }
            }

            return results;
        }

        std::vector<std::pair<std::string, int>> searchAdd(const std::string& query){
            std::vector<std::pair<std::string, int>> results;
            std::istringstream iss(query);
            std::string word;
            while (iss >> word){
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                if (word[0] == addSign){
                    word = word.substr(1);
                    if (filesMap.find(word) != filesMap.end()){
                        for (const auto& doc : filesMap[word]){
                            results.push_back({doc.getDocumentName(), doc.getFrequency()});
                        }
                    }
                }
            }
            return results;
        }

        std::vector<std::pair<std::string, int>> searchSub(const std::string& query){
            std::vector<std::pair<std::string, int>> results;
            std::istringstream iss(query);
            std::string word;
            while (iss >> word){
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                if (word[0] == subSign){
                    word = word.substr(1);
                    if (filesMap.find(word) != filesMap.end()){
                        for (const auto& doc : filesMap[word]){
                            results.push_back({doc.getDocumentName(), doc.getFrequency()});
                        }
                    }
                }
            }
            return results;
        }

        std::vector<std::pair<std::string, int>> searchSentence(const std::string& query) {
            std::istringstream iss(query);
            std::string word;
            std::unordered_map<std::string, int> documentScores; // Map to track document scores
            std::unordered_map<std::string, std::unordered_set<std::string>> documentWordMap; // Track words in each doc
            std::unordered_map<std::string, int> wordCount; // Count occurrences of each word
            std::vector<std::string> words;
            while (iss >> word) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                words.push_back(word);
                wordCount[word]++;
            }
            for (const auto& queryWord : words) {
                if (filesMap.find(queryWord) != filesMap.end()) {
                    for (const auto& doc : filesMap[queryWord]) {
                        documentScores[doc.getDocumentName()] += doc.getFrequency();
                        documentWordMap[doc.getDocumentName()].insert(queryWord);
                    }
                }
            }
            std::vector<std::pair<std::string, int>> results;
            for (const auto& [docName, score] : documentScores) {
                results.push_back({docName, score});
            }
            std::sort(results.begin(), results.end(), [&](const auto& a, const auto& b) {
                size_t wordsA = documentWordMap[a.first].size();
                size_t wordsB = documentWordMap[b.first].size();
                if (wordsA != wordsB) {
                    return wordsA > wordsB;
                }
                return a.second > b.second;
            });
            return results;
        }
    public:
        searchEngine() = default;
        void indexFiles(){
            std::vector<std::string> files;
            for (const auto& entry : fs::directory_iterator(mainDir)){
                files.push_back(entry.path().string());
            }
            for (const auto& file : files){
                std::ifstream fin(file);
                std::string line;
                int pos = 0;
                while (std::getline(fin, line)){
                    std::istringstream iss(line);
                    std::string word;
                    while (iss >> word){
                        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                        if (filesMap.find(word) == filesMap.end()){
                            filesMap[word].push_back(wordInDocument(file, pos));
                        } else {
                            if (std::find(filesMap[word].begin(), filesMap[word].end(), wordInDocument(file)) == filesMap[word].end()){
                                filesMap[word].push_back(wordInDocument(file, pos));
                            } else {
                                for (auto& wordDoc : filesMap[word]){
                                    if (wordDoc.appearsInDocument(file)){
                                        wordDoc.addPosition(pos);
                                    }
                                }
                            }
                        }
                        pos++;
                    }
                }
            }
        }
        void search(const std::string& query) {
            searchType type;
            queryType(query, type);
            std::string types[] = {"defaultSearch", "addSearch", "subSearch", "sentenceSearch", "invalidSearch"};
            std::cout << "Type: " << types[type] << std::endl;
            std::vector<std::pair<std::string, int>> results;
            switch (type) {
                case defaultSearch: results = searchDefault(query); break;
                case addSearch: results = searchAdd(query); break;
                case subSearch: results = searchSub(query); break;
                case sentenceSearch: results = searchSentence(query); break;
                case invalidSearch: { std::cout << "Invalid search query\n"; return; } break;
            }
        
            if (results.empty()) std::cout << "No results found\n";
            else for (const auto& result : results) { std::cout << result.first << "   " << result.second << std::endl; }
        }
        ~searchEngine() = default;
        // for debugging purposes
        void printFilesMap(){
            for (const auto& [word, doc] : filesMap){
                std::cout << word << ":\n";
            }
        }
};

int main() {
    searchEngine se;
    se.indexFiles();
    while(true){
        std::string query;
        std::cout << "Enter query: ";
        std::getline(std::cin, query);
        if (query == "exit") break;
        se.search(query);
    }
    return 0;
}