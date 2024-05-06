import json
import os

def read_jokes_from_file(file_path):
    with open(file_path, 'r') as file:
        jokes = json.load(file)
    return ["{} {}".format(joke.get('title', ''), joke.get('body', '')) for joke in jokes]

def split_jokes(jokes, n):
    return [jokes[i:i + n] for i in range(0, len(jokes), n)]

def write_jokes_to_files(jokes, base_file_name):
    for i, jokes_chunk in enumerate(jokes):
        with open('{}_{}.txt'.format(base_file_name, i+1), 'w') as file:
            file.write('\n'.join(jokes_chunk))

file_names = ['reddit_jokes.json', 'stupidstuff.json', 'wocka.json']
all_jokes = []

for file_name in file_names:
    all_jokes.extend(read_jokes_from_file(file_name))

jokes_chunks = split_jokes(all_jokes, 1000)

write_jokes_to_files(jokes_chunks, 'article')