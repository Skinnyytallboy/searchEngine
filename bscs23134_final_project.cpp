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
#include <climits>
#include <queue>

// define namespace and alias
namespace fs = std::filesystem;

// Signs that will/can be used while searching
#define addSign '+'
#define subSign '-'
#define spcSign ' '
#define sentenceSign '"'

// Folder name which contains all the files
#define mainDir1 "review_text"
#define mainDir2 "tempFolder"
std::string mainDir = mainDir1;

// Enum to store the type of search query
enum searchType { defaultSearch, addSearch, subSearch, sentenceSearch, sentenceSubSearch, invalidSearch };

// Class to store the word and its position in the document
class wordInDocument{
    private:
        std::string documentName;
        std::string content;
        std::vector<int> positions;
    public:
		wordInDocument(const std::string& docName) : documentName(docName) {}
        wordInDocument(const std::string& docName, const std::string& docContent) : documentName(docName), content(docContent) {}
        wordInDocument(const std::string& docName, const std::string& docContent, int pos) : documentName(docName), content(docContent) { positions.push_back(pos); }
        wordInDocument(const std::string& docName, int pos) : documentName(docName) { positions.push_back(pos); }
    	void setContent(const std::string& cont) { content = cont; }
        std::string getContent() const { return content; }
        void addPosition(int pos) { positions.push_back(pos); }
    	std::string getDocumentName() const { return documentName; }
    	const std::vector<int>& getPositions() const { return positions; }
    	int getFrequency() const { return positions.size(); }
        bool appearsInDocument(const std::string& docName) const { return documentName == docName; }
        bool appearsInPosition(int pos) const { return std::find(positions.begin(), positions.end(), pos) != positions.end(); }
        bool operator==(const wordInDocument& other) const { return documentName == other.documentName; }
        ~wordInDocument() = default;
};

class searchEngineUnordered {
    private:
        std::unordered_map<std::string, std::vector<wordInDocument>> filesMap; 
        
        void queryType(const std::string& query, searchType& type) {
            type = invalidSearch;
            std::string tempQuery = query;
            std::transform(tempQuery.begin(), tempQuery.end(), tempQuery.begin(), ::tolower);
            if (tempQuery.find(addSign) != std::string::npos) type = addSearch;
            else if (tempQuery.find(sentenceSign) != std::string::npos) {
                if (tempQuery.find(subSign) != std::string::npos) type = sentenceSubSearch;
                else type = sentenceSearch;
            }
            else if (tempQuery.find(subSign) != std::string::npos) type = subSearch;
            else type = defaultSearch;
        }
        
        std::vector<std::pair<std::string, int>> searchDefault(const std::string& query) {
            std::istringstream iss(query);
            std::string word;
            std::unordered_map<std::string, int> documentScores; 
            std::unordered_map<std::string, std::unordered_set<std::string>> documentWordMap;
            std::unordered_map<std::string, int> wordCount;
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
            for (const auto& [docName, score] : documentScores) results.push_back({docName, score});
            std::sort(results.begin(), results.end(), [&](const auto& a, const auto& b) {
                size_t wordsA = documentWordMap[a.first].size();
                size_t wordsB = documentWordMap[b.first].size();
                if (wordsA != wordsB) return wordsA > wordsB;
                return a.second > b.second;
            });
            return results;
        }

        std::vector<std::pair<std::string, int>> searchAdd(const std::string& query) {
            std::vector<std::pair<std::string, int>> results;
            std::istringstream iss(query);
            std::string word;
            std::vector<std::string> words;
            while (std::getline(iss, word, addSign)) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                words.push_back(word);
            }
            std::unordered_map<std::string, std::vector<int>> documentOccurrences;
            for (const auto& word : words) {
                if (filesMap.find(word) != filesMap.end()) {
                    for (const auto& doc : filesMap[word]) {
                        const std::string& docName = doc.getDocumentName();
                        if (documentOccurrences.find(docName) == documentOccurrences.end()) documentOccurrences[docName] = std::vector<int>(words.size(), 0);
                        auto it = std::find(words.begin(), words.end(), word);
                        int wordIndex = std::distance(words.begin(), it);
                        documentOccurrences[docName][wordIndex] = doc.getFrequency();
                    }
                }
            }
            for (const auto& [docName, freqs] : documentOccurrences) {
                bool containsAllWords = true;
                int minOccurrences = INT_MAX;
                for (const auto& word : words) {
                    bool wordFoundInDoc = false;
                    int wordFrequency = 0;
                    for (const auto& doc : filesMap[word]) {
                        if (doc.getDocumentName() == docName) {
                            wordFoundInDoc = true;
                            wordFrequency = doc.getFrequency();
                            break;
                        }
                    }
                    if (!wordFoundInDoc) { containsAllWords = false; break; }
                    minOccurrences = std::min(minOccurrences, wordFrequency);
                }
                if (containsAllWords) results.push_back({docName, minOccurrences});
            }
            std::sort(results.begin(), results.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
            return results;
        }

        std::vector<std::pair<std::string, int>> searchSub(const std::string& query) {
            std::vector<std::pair<std::string, int>> results;
            std::istringstream iss(query);
            std::string word;
            std::vector<std::string> words;
            while (std::getline(iss, word, subSign)) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);  // Convert to lowercase
                words.push_back(word);
            }
            std::string word1 = words[0]; 
            std::vector<std::string> excludeWords(words.begin() + 1, words.end());
            if (filesMap.find(word1) != filesMap.end()) {
                for (const auto& doc : filesMap[word1]) {
                    const std::string& docName = doc.getDocumentName();
                    bool containsAnyExcludeWord = false;
                    for (const auto& excludeWord : excludeWords) {
                        if (filesMap.find(excludeWord) != filesMap.end()) {
                            for (const auto& doc2 : filesMap[excludeWord]) if (doc2.getDocumentName() == docName) { containsAnyExcludeWord = true; break; }
                        }
                        if (containsAnyExcludeWord) break;
                    }
                    if (!containsAnyExcludeWord) results.push_back({docName, doc.getFrequency()});
                }
            }
            std::sort(results.begin(), results.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
            return results;
        }

        std::vector<std::pair<std::string, int>> searchSentence(const std::string& query) {
            std::vector<std::pair<std::string, int>> results;
            std::string sentence = query.substr(1, query.size() - 2);
            std::istringstream iss(sentence);
            std::string word;
            std::vector<std::string> words;
            while (iss >> word) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                words.push_back(word);
            }
            std::unordered_map<std::string, std::vector<int>> documentOccurrences;
            for (const auto& word : words) {
                if (filesMap.find(word) != filesMap.end()) {
                    for (const auto& doc : filesMap[word]) {
                        const std::string& docName = doc.getDocumentName();
                        if (documentOccurrences.find(docName) == documentOccurrences.end()) documentOccurrences[docName] = std::vector<int>(words.size(), 0);
                        auto it = std::find(words.begin(), words.end(), word);
                        int wordIndex = std::distance(words.begin(), it);
                        documentOccurrences[docName][wordIndex] = doc.getFrequency();
                    }
                }
            }
            for (const auto& [docName, freqs] : documentOccurrences) {
                bool containsAllWords = true;
                int minOccurrences = INT_MAX;
                for (const auto& word : words) {
                    bool wordFoundInDoc = false;
                    int wordFrequency = 0;
                    for (const auto& doc : filesMap[word]) {
                        if (doc.getDocumentName() == docName) {
                            wordFoundInDoc = true;
                            wordFrequency = doc.getFrequency();
                            break;
                        }
                    }
                    if (!wordFoundInDoc) { containsAllWords = false; break; }
                    minOccurrences = std::min(minOccurrences, wordFrequency);
                }
                if (containsAllWords) results.push_back({docName, minOccurrences});
            }
            std::sort(results.begin(), results.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
            return results;
        }

        std::vector<std::pair<std::string, int>> searchSentenceSub(const std::string& query) {
            std::vector<std::pair<std::string, int>> finalResults, results;
            std::istringstream queryStream(query);
            std::string sentence;
            std::queue<std::string> sentences;
            while (std::getline(queryStream, sentence, '-')) {
                sentence.erase(0, sentence.find_first_not_of(" \t"));
                sentence.erase(sentence.find_last_not_of(" \t") + 1);
                sentences.push(sentence);
            }
            bool haveToCheckFirst = true;
            while(!sentences.empty()){
                if (haveToCheckFirst){
                    haveToCheckFirst = false;
                    std::string firstSentence = sentences.front();
                    sentences.pop();
                    finalResults = searchSentence(firstSentence);
                    results = finalResults;
                } else {
                    std::string sentence = sentences.front();
                    sentences.pop();
                    std::vector<std::pair<std::string, int>> resultsFromOtherSentence = searchSentence(sentence);
                    for (const auto& result : results){
                        if (std::find(resultsFromOtherSentence.begin(), resultsFromOtherSentence.end(), result) == resultsFromOtherSentence.end()){
                            results.erase(std::remove(results.begin(), results.end(), result), results.end());
                        }
                    }
                }
            }
            for (const auto& result : results) {
                if (std::find(finalResults.begin(), finalResults.end(), result) != finalResults.end()) 
                    finalResults.erase(std::remove(finalResults.begin(), finalResults.end(), result), finalResults.end()); 
            }
            return finalResults;
        }
        
        void indexFiles() {
            std::vector<std::string> files;
            for (const auto& entry : fs::directory_iterator(mainDir)) files.push_back(entry.path().string());
            for (const auto& file : files) {
                std::ifstream fin(file);
                std::string line, content;
                int pos = 0;
                while (std::getline(fin, line)) {
                    content += line + " ";
                    std::istringstream iss(line);
                    std::string word;
                    while (iss >> word) {
                        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                        if (filesMap.find(word) == filesMap.end()) 
                            filesMap[word].push_back(wordInDocument(file, content, pos));
                        else {
                            if (std::find(filesMap[word].begin(), filesMap[word].end(), wordInDocument(file, content)) == filesMap[word].end()) 
                                filesMap[word].push_back(wordInDocument(file, content, pos));
                            else for (auto& wordDoc : filesMap[word]) if (wordDoc.appearsInDocument(file)) wordDoc.addPosition(pos);
                        }
                        pos++;
                    }
                }
            }
        }

        void search(const std::string& query) {
            searchType type;
            queryType(query, type);
            std::string types[] = {"defaultSearch", "addSearch", "subSearch", "sentenceSearch","sentenceSubSearch", "invalidSearch"};
            std::cout << "Type: " << types[type] << std::endl;
            std::vector<std::pair<std::string, int>> results;
            switch (type) {
                case defaultSearch: results = searchDefault(query); break;
                case addSearch: results = searchAdd(query); break;
                case subSearch: results = searchSub(query); break;
                case sentenceSearch: results = searchSentence(query); break;
                case sentenceSubSearch: results = searchSentenceSub(query); break;
                case invalidSearch: { std::cout << "Invalid search query\n"; return; } break;
            }
        
            if (results.empty()) std::cout << "No results found\n";
            else for (const auto& result : results) { std::cout << result.first << "   " << result.second << std::endl; }
        }

    public:
        searchEngineUnordered() { this->indexFiles(); }
        
        void engine() {
            while(true){
                std::string query;
                std::cout << "Enter query: ";
                std::getline(std::cin, query);
                if (query == "exit") break;
                this->search(query);
            }
        }
        
        ~searchEngineUnordered() = default;
};

struct trieNode {
    std::unordered_map<char, trieNode*> children;
    std::vector<wordInDocument> occurrences;
    bool isEndOfWord;

    trieNode() : isEndOfWord(false) {}
};

class trie{
    private:
        trieNode* root;

        void clear(trieNode* node) {
            if (!node) return;
            for (auto& child : node->children) clear(child.second);
            delete node;
        }

    public:
        trie() { root = new trieNode(); }

        void insert(const std::string& word, const std::string& docName, const std::string& docContent, int pos) {
            wordInDocument wordDoc(docName, docContent, pos);
            trieNode* current = root;
            for (const auto& letter : word) {
                if (current->children.find(letter) == current->children.end()) current->children[letter] = new trieNode();
                current = current->children[letter];
            }
            current->occurrences.push_back(wordDoc);
            current->isEndOfWord = true;
        }

        std::vector<wordInDocument> search(const std::string& word) {
            trieNode* current = root;
            for (const auto& letter : word) {
                if (current->children.find(letter) == current->children.end()) return {};
                current = current->children[letter];
            }
            return current->isEndOfWord ? current->occurrences : std::vector<wordInDocument>();
        }

        ~trie() { this->clear(root); }
};

class searchEngineTries{
    private:
        trie trieObj;

        void indexFiles() {
            std::vector<std::string> files;
            for (const auto& entry : fs::directory_iterator(mainDir)) files.push_back(entry.path().string());
            for (const auto& file : files) {
                std::ifstream fin(file);
                std::string line, content;
                int pos = 0;
                while (std::getline(fin, line)) {
                    content += line + " ";
                    std::istringstream iss(line);
                    std::string word;
                    while (iss >> word) {
                        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                        trieObj.insert(word, file, content, pos);
                        pos++;
                    }
                }
            }
        }

        void queryType(const std::string& query, searchType& type) {
            type = invalidSearch;
            std::string tempQuery = query;
            std::transform(tempQuery.begin(), tempQuery.end(), tempQuery.begin(), ::tolower);
            if (tempQuery.find(addSign) != std::string::npos) type = addSearch;
            else if (tempQuery.find(sentenceSign) != std::string::npos) {
                if (tempQuery.find(subSign) != std::string::npos) type = sentenceSubSearch;
                else type = sentenceSearch;
            }
            else if (tempQuery.find(subSign) != std::string::npos) type = subSearch;
            else type = defaultSearch;
        }

        std::vector<std::pair<std::string, int>> searchDefault(const std::string& query) {
            std::istringstream iss(query);
            std::string word;
            std::unordered_map<std::string, int> documentScores; 
            std::unordered_map<std::string, std::unordered_set<std::string>> documentWordMap;
            std::unordered_map<std::string, int> wordCount;
            std::vector<std::string> words;
            while (iss >> word) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                words.push_back(word);
                wordCount[word]++;
            }
            for (const auto& queryWord : words) {
                std::vector<wordInDocument> occurrences = trieObj.search(queryWord);
                for (const auto& doc : occurrences) {
                    documentScores[doc.getDocumentName()] += doc.getFrequency();
                    documentWordMap[doc.getDocumentName()].insert(queryWord);
                }
            }
            std::vector<std::pair<std::string, int>> results;
            for (const auto& [docName, score] : documentScores) results.push_back({docName, score});
            std::sort(results.begin(), results.end(), [&](const auto& a, const auto& b) {
                size_t wordsA = documentWordMap[a.first].size();
                size_t wordsB = documentWordMap[b.first].size();
                if (wordsA != wordsB) return wordsA > wordsB;
                return a.second > b.second;
            });
            return results;
        }

        std::vector<std::pair<std::string, int>> searchAdd(const std::string& query) {
            std::vector<std::pair<std::string, int>> results;
            std::istringstream iss(query);
            std::string word;
            std::vector<std::string> words;
            while (std::getline(iss, word, addSign)) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                words.push_back(word);
            }
            std::unordered_map<std::string, std::vector<int>> documentOccurrences;
            for (const auto& word : words) {
                std::vector<wordInDocument> occurrences = trieObj.search(word);
                for (const auto& doc : occurrences) {
                    const std::string& docName = doc.getDocumentName();
                    if (documentOccurrences.find(docName) == documentOccurrences.end()) documentOccurrences[docName] = std::vector<int>(words.size(), 0);
                    auto it = std::find(words.begin(), words.end(), word);
                    int wordIndex = std::distance(words.begin(), it);
                    documentOccurrences[docName][wordIndex] = doc.getFrequency();
                }
            }
            for (const auto& [docName, freqs] : documentOccurrences) {
                bool containsAllWords = true;
                int minOccurrences = INT_MAX;
                for (const auto& word : words) {
                    std::vector<wordInDocument> occurrences = trieObj.search(word);
                    bool wordFoundInDoc = false;
                    int wordFrequency = 0;
                    for (const auto& doc : occurrences) {
                        if (doc.getDocumentName() == docName) {
                            wordFoundInDoc = true;
                            wordFrequency = doc.getFrequency();
                            break;
                        }
                    }
                    if (!wordFoundInDoc) { containsAllWords = false; break; }
                    minOccurrences = std::min(minOccurrences, wordFrequency);
                }
                if (containsAllWords) results.push_back({docName, minOccurrences});
            }
            std::sort(results.begin(), results.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
            return results;
        }

        std::vector<std::pair<std::string, int>> searchSub(const std::string& query) {
            std::vector<std::pair<std::string, int>> results;
            std::istringstream iss(query);
            std::string word;
            std::vector<std::string> words;
            while (std::getline(iss, word, subSign)) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);  // Convert to lowercase
                words.push_back(word);
            }
            std::string word1 = words[0]; 
            std::vector<std::string> excludeWords(words.begin() + 1, words.end());
            std::vector<wordInDocument> occurrences = trieObj.search(word1);
            for (const auto& doc : occurrences) {
                const std::string& docName = doc.getDocumentName();
                bool containsAnyExcludeWord = false;
                for (const auto& excludeWord : excludeWords) {
                    std::vector<wordInDocument> excludeOccurrences = trieObj.search(excludeWord);
                    bool excludeWordFound = false;
                    for (const auto& doc2 : excludeOccurrences) if (doc2.getDocumentName() == docName) { excludeWordFound = true; break; }
                    if (excludeWordFound) { containsAnyExcludeWord = true; break; }
                }
                if (!containsAnyExcludeWord) results.push_back({docName, doc.getFrequency()});
            }
            std::sort(results.begin(), results.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
            return results;
        }

        std::vector<std::pair<std::string, int>> searchSentence(const std::string& query) {
            std::vector<std::pair<std::string, int>> results;
            std::string sentence = query.substr(1, query.size() - 2);
            std::istringstream iss(sentence);
            std::string word;
            std::vector<std::string> words;
            while (iss >> word) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                words.push_back(word);
            }
            std::unordered_map<std::string, std::vector<int>> documentOccurrences;
            for (const auto& word : words) {
                std::vector<wordInDocument> occurrences = trieObj.search(word);
                for (const auto& doc : occurrences) {
                    const std::string& docName = doc.getDocumentName();
                    if (documentOccurrences.find(docName) == documentOccurrences.end()) documentOccurrences[docName] = std::vector<int>(words.size(), 0);
                    auto it = std::find(words.begin(), words.end(), word);
                    int wordIndex = std::distance(words.begin(), it);
                    documentOccurrences[docName][wordIndex] = doc.getFrequency();
                }
            }
            for (const auto& [docName, freqs] : documentOccurrences) {
                bool containsAllWords = true;
                int minOccurrences = INT_MAX;
                for (const auto& word : words) {
                    std::vector<wordInDocument> occurrences = trieObj.search(word);
                    bool wordFoundInDoc = false;
                    int wordFrequency = 0;
                    for (const auto& doc : occurrences) {
                        if (doc.getDocumentName() == docName) {
                            wordFoundInDoc = true;
                            wordFrequency = doc.getFrequency();
                            break;
                        }
                    }
                    if (!wordFoundInDoc) { containsAllWords = false; break; }
                    minOccurrences = std::min(minOccurrences, wordFrequency);
                }
                if (containsAllWords) results.push_back({docName, minOccurrences});
            }
            std::sort(results.begin(), results.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
            return results;
        }

        std::vector<std::pair<std::string, int>> searchSentenceSub(const std::string& query) {
            std::vector<std::pair<std::string, int>> finalResults, results;
            std::istringstream queryStream(query);
            std::string sentence;
            std::queue<std::string> sentences;
            while (std::getline(queryStream, sentence, '-')) {
                sentence.erase(0, sentence.find_first_not_of(" \t"));
                sentence.erase(sentence.find_last_not_of(" \t") + 1);
                sentences.push(sentence);
            }
            bool haveToCheckFirst = true;
            while(!sentences.empty()){
                if (haveToCheckFirst){
                    haveToCheckFirst = false;
                    std::string firstSentence = sentences.front();
                    sentences.pop();
                    finalResults = searchSentence(firstSentence);
                    results = finalResults;
                } else {
                    std::string sentence = sentences.front();
                    sentences.pop();
                    std::vector<std::pair<std::string, int>> resultsFromOtherSentence = searchSentence(sentence);
                    for (const auto& result : results){
                        if (std::find(resultsFromOtherSentence.begin(), resultsFromOtherSentence.end(), result) == resultsFromOtherSentence.end()){
                            results.erase(std::remove(results.begin(), results.end(), result), results.end());
                        }
                    }
                }
            }
            for (const auto& result : results) {
                if (std::find(finalResults.begin(), finalResults.end(), result) != finalResults.end()) 
                    finalResults.erase(std::remove(finalResults.begin(), finalResults.end(), result), finalResults.end()); 
            }
            return finalResults;
        }
        
        void search(const std::string& query) {
            searchType type;
            queryType(query, type);
            std::string types[] = {"defaultSearch", "addSearch", "subSearch", "sentenceSearch","sentenceSubSearch", "invalidSearch"};
            std::cout << "Type: " << types[type] << std::endl;
            std::vector<std::pair<std::string, int>> results;
            switch (type) {
                case defaultSearch: results = searchDefault(query); break;
                case addSearch: results = searchAdd(query); break;
                case subSearch: results = searchSub(query); break;
                case sentenceSearch: results = searchSentence(query); break;
                case sentenceSubSearch: results = searchSentenceSub(query); break;
                case invalidSearch: { std::cout << "Invalid search query\n"; return; } break;
            }
        
            if (results.empty()) std::cout << "No results found\n";
            else for (const auto& result : results) { std::cout << result.first << "   " << result.second << std::endl; }
        }

    public:
        searchEngineTries() { this->indexFiles(); }

        void engine() {
            while(true){
                std::string query;
                std::cout << "Enter query: ";
                std::getline(std::cin, query);
                if (query == "exit") break;
                this->search(query);
            }
        }

        ~searchEngineTries() = default;
};

void wholeProject() {
    std::cout << "Welcome to the Search Engine\n";
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Enter 1 for Unordered Map Search Engine\n";
    std::cout << "Enter 2 for Trie Search Engine\n";
    std::cout << "Enter 'exit' to exit the program\n";
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "1") {
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << "Enter 'temp' to search in the temporary folder\n";
        std::cout << "Enter 'review' to search in the review folder\n";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "temp" || choice == "Temp") mainDir = mainDir2;
        else if (choice == "review" || choice == "Review") mainDir = mainDir1;
        else {
            std::cout << "Invalid choice\n";
            wholeProject();
        }
        searchEngineUnordered searchEngine;
        searchEngine.engine();
    } else if (choice == "2") {
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << "Enter 'temp' to search in the temporary folder\n";
        std::cout << "Enter 'review' to search in the review folder\n";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "temp" || choice == "Temp") mainDir = mainDir2;
        else if (choice == "review" || choice == "Review") mainDir = mainDir1;
        else {
            std::cout << "Invalid choice\n";
            wholeProject();
        }
        searchEngineTries searchEngine;
        searchEngine.engine();
    } else if (choice == "exit") {
        std::cout << "Exiting the program\n";
    } else {
        std::cout << "Invalid choice\n";
        wholeProject();
    }
}

int main() { wholeProject(); return 0; }