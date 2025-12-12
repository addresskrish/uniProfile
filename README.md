# 🔍 UniProfile

**A fast, lightweight, and secure OSINT tool written in C to find user profiles across multiple social media platforms.**

![Language](https://img.shields.io/badge/language-C-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)

## 📖 Overview

UniProfile is a command-line interface (CLI) tool designed for Open Source Intelligence (OSINT) gathering. It scans popular social media platforms (GitHub, Reddit, Twitter, etc.) to determine if a specific username exists.

Unlike complex Python scripts that require heavy dependencies, UniProfile runs on pure **C** and system-native **cURL**, making it portable and incredibly fast.

## ✨ Features

* **🛡️ Secure Input:** Input sanitization prevents command injection attacks.
* **🚀 Cross-Platform:** Works seamlessly on Windows, Linux, and macOS.
* **💾 Save Results:** Export found profiles to `results.txt` for reporting.
* **🎨 Interactive UI:** Beautiful ASCII interface with color-coded status (CONFIRMED vs MAYBE).
* **🔗 Browser Integration:** Open found profiles directly in your default browser from the CLI.
* **🕵️ Stealth:** Uses a real browser User-Agent to avoid simple bot detection.

## 🛠️ Prerequisites

To run this tool, you need:

1.  **GCC Compiler** (to compile the C code).
2.  **cURL** installed and accessible in your system path (Terminal/Command Prompt).

## 📥 Installation & Compilation

### Linux / macOS

1.  **Install dependencies** (if not already installed):
    ```bash
    # Ubuntu/Debian
    sudo apt install build-essential curl

    # macOS
    brew install curl
    ```

2.  **Compile the code:**
    ```bash
    gcc main.c -o uniprofile
    ```

3.  **Run:**
    ```bash
    ./uniprofile
    ```

### Windows

1.  Ensure you have **MinGW** installed (for `gcc`).
2.  Ensure **curl** is installed (Windows 10/11 has it by default).
3.  **Compile:**
    ```powershell
    gcc main.c -o uniprofile.exe
    ```
4.  **Run:**
    ```powershell
    .\uniprofile.exe
    ```

## 🖥️ Usage

1.  Start the application.
2.  Enter the target **username** when prompted.
3.  Wait for the scan to complete (Green = Found, Red = Not Found, Yellow = Maybe/Blocked).
4.  Use the command menu:
    * **`id`** (e.g., `1`, `5`): Open that specific profile in your browser.
    * **`all`**: Open ALL found profiles at once.
    * **`save`**: Save the scan results to `results.txt`.
    * **`exit`**: Close the tool.

## 📋 Supported Platforms

Currently scans 13+ major platforms including:
- GitHub
- Reddit
- StackOverflow
- GitLab
- Medium
- YouTube
- Pinterest
- Vimeo
- And more...

## ⚠️ Disclaimer

This tool is for **educational purposes and security research only**. The author is not responsible for any misuse of this tool. Do not use this to harass or stalk individuals.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1.  Fork the Project
2.  Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3.  Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4.  Push to the Branch (`git push origin feature/AmazingFeature`)
5.  Open a Pull Request

---
**Created by [Krish](https://x.com/addresskrish)**