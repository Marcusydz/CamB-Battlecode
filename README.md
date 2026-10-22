# Cambridge Battlecode Bot (C++)

- Author: Mac
- Date: 22/10/2026

## Useful Git Commands

- ``git pull``
- ``git pull origin main``
- ``git add [FILES]`` - Track the files
- ``git add .`` - Track all changed files
- ``git commit -m "MESSAGE"`` - Commit your changes
- ``git push`` - Push your changes on GitHub
- ``git status`` - Status of your git, such as commited, file changes, file tracked.

## LLM Guide

The bot ``main`` is the main bot for this Battlecode. All versionings guides should refers to [here](/VERSIONS.md). Other than the markdown file, each version should have its own markdown file inside ``/docs/versions`` for explaination and change-logs, including the author AI model, date, next recommended steps.

### Scripts

There are different python or shell scripts that is in ``scripts/``, possibly useful for testing and unranked matches generation. The ``cambc`` CLI is used to interact with the server, it should be either installed globally on shell or locally in the python directory.

### Git Operations

In Git, despite the default branch is not main, branch main remains as our main branch, each version release should also be pushed as a tag with the name of the version.