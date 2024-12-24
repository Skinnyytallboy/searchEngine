import csv
import os
import time
from pathlib import Path

csv_file_path = "Reviews.csv"
output_folder = Path("review_text")

def create_review_text_file(review):
    file_name = output_folder / f"review_{review['Id']}.txt"
    if not file_name.exists():
        with open(file_name, 'w', encoding='utf-8') as file:
            file.write(f"ProductId: {review['ProductId']}\n")
            file.write(f"UserId: {review['UserId']}\n")
            file.write(f"ProfileName: {review['ProfileName']}\n")
            file.write(f"HelpfulnessNumerator: {review['HelpfulnessNumerator']}\n")
            file.write(f"HelpfulnessDenominator: {review['HelpfulnessDenominator']}\n")
            file.write(f"Score: {review['Score']}\n")
            file.write(f"Time: {review['Time']}\n")
            file.write(f"Summary: {review['Summary']}\n")
            file.write(f"Text: {review['Text']}\n")
        print(f"Created file: {file_name}")
    else:
        print(f"File {file_name} already exists. Skipping.")

def csv_to_text(csv_file_path):
    output_folder.mkdir(exist_ok=True)
    print("Starting conversion from CSV to text files...")

    start_time = time.time()

    try:
        with open(csv_file_path, 'r', encoding='utf-8') as csv_file:
            reader = csv.DictReader(csv_file)
            total_reviews = sum(1 for _ in reader)
            csv_file.seek(0)
            next(reader)

            for index, row in enumerate(reader, start=1):
                create_review_text_file(row)
                elapsed_time = time.time() - start_time
                print(f"Processed review {index}/{total_reviews} - Elapsed Time: {elapsed_time:.2f} seconds")

        total_time = time.time() - start_time
        print(f"\nConversion complete. Text files created for each review in the 'review_text' folder.")
        print(f"Total Time Taken: {total_time:.2f} seconds")
        
    except FileNotFoundError:
        print(f"The file {csv_file_path} does not exist.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    csv_to_text(csv_file_path)
