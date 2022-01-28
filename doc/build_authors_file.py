#!/usr/bin/env python3
# Automatically generates AUTHORS.md from GitHub API data
from github import Github

REPO = "libui-ng/libui-ng"
HEADER = "**The libui-ng project would like to acknowledge the following contributors:**"
FILE = "AUTHORS.md"

def fetch_authors(repo):
    # fully load the paginated list before processing
    return [author for author in repo.get_contributors()]

def write_authors_file(authors, f):
    f.write("# Authors\n\n")
    f.write(HEADER + "\n\n")
    for author in authors:
        handle = "@" + author.login
        name = author.name
        if name is not None and len(name.split(" ")) > 1:
            name += " "
        else:
            name = ""
        f.write(" * " + name + handle + "\n")

def main():
    g = Github()
    repo = g.get_repo(REPO)
    with open(FILE, "w") as f:
        write_authors_file(fetch_authors(repo), f)

if __name__ == "__main__":
    main()