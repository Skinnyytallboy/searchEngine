# Search Engine Project  

## Introduction  
This project is a high-performance search engine designed to scan and retrieve data from nearly 500,000 files stored in a directory named `review_txt`. It supports multiple query styles and has two implementation variants:
- **Unordered Map-based Variant**
- **Trie-based Variant**

## Features  
- **Efficient Data Scanning**: Handles a large volume of files seamlessly.  
- **Multiple Query Types**:
  - `defaultSearch`: Basic search.  
  - `addSearch`: Search with additional terms.  
  - `subSearch`: Search excluding specific terms.  
  - `sentenceSearch`: Searches for an exact sentence.  
  - `sentenceSubSearch`: Sentence search with exclusions.  
  - `invalidSearch`: Handles invalid queries gracefully.  
- **Results Display**: Outputs relevant file results based on the query.


## How It Works  
1. The engine scans the `review_txt` directory and loads the files into memory.  
2. It processes queries based on the query type provided by the user.  
3. Results are displayed, showing matching files and relevant content snippets.  

## Implementations  
### **Unordered Map-based Variant**  
This variant uses C++'s `std::unordered_map` for data storage and retrieval. It is optimized for fast key-based lookups.  

### **Trie-based Variant**  
This variant uses a Trie (prefix tree) structure for efficient string matching, especially for sentence and prefix-based queries.  

## Performance  
- Designed to handle high query loads with minimal latency.  
- Benchmarked for scalability with 500,000+ files.  

## Requirements  
- **C++ Compiler**: GCC 11+ or MSVC 2022+ recommended.  
- **Operating System**: Linux (Arch preferred) or Windows.  
- **Memory**: Minimum 8 GB for optimal performance.  

## Installation and Usage  
### **1. Clone the Repository**  
```bash  
git clone https://github.com/your-username/search-engine.git
cd search-engine 
```
### **2. Download the Files**
Run the python file 'fileExtractScript.py' which will downlaod all the required files and make a direcotory for your program. You can also make a 'tempFolder' directory with less number of files.
### **3. Build the Project**
```bash
make  
# or manually  
g++ -std=c++17 -O2 -o search_engine src/*.cpp  
```
### **4.  Run the Engine**
```bash
./search_engine  
```

## Provide Queries
The engine will prompt for query input. Use the following formats:
-    defaultSearch: Enter terms to search.
-    addSearch: Add + before terms to include.
-    subSearch: Add - before terms to exclude.
-    sentenceSearch: Enter a full sentence in quotes.
-    sentenceSubSearch: Use quotes and - for exclusions

### License
This project is licensed under the MIT License.
Author

Jalal Ahmed  
Lahore, Pakistan
GitHub: Jalal Ahmed (Skinnyytallboy)
