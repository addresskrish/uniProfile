# UniProfile – A Social Media OSINT Tool

#### Video Demo: https://youtu.be/YvpB1kDHxWg

#### Description:

UniProfile is a command-line OSINT (Open Source Intelligence) tool written in C that checks whether a given username exists across multiple popular social media platforms. The goal of this project is to provide a fast, lightweight, and dependency-minimal alternative to existing username enumeration tools, while also focusing on security, portability, and clean terminal output.

The program takes a username as input, validates it to prevent command injection attacks, and then scans multiple social media websites such as GitHub, Reddit, GitLab, Medium, Dev.to, YouTube, LinkedIn, Twitter/X, Instagram, and Facebook. For each platform, UniProfile constructs the appropriate profile URL and checks its existence using HTTP status codes retrieved via `curl`.

This project was built as my final submission for **CS50x**, and it reflects my interest in systems programming, security, and OSINT-style tooling.

---

## How the Project Works

When the program starts, it displays an ASCII banner and prompts the user to enter a username. Before performing any network requests, the username is sanitized to ensure it contains only safe characters (letters, numbers, dots, hyphens, and underscores). This prevents malicious command injection when the username is passed to system commands.

Once validated, UniProfile scans a predefined list of platforms. For each site:

- A profile URL is generated using a format string.
- A request is made using `curl` with:
  - A timeout to prevent hanging connections
  - A common browser User-Agent to reduce bot blocking
  - Silent mode to suppress unnecessary output
- The HTTP status code is captured and analyzed.

A progress bar is displayed in real time while sites are being scanned, giving immediate feedback to the user.

---

## Accurate Detection Strategy

Some platforms return misleading HTTP responses. To address this:

- **GitHub** is checked using its official public API (`api.github.com/users/{username}`), which provides accurate status codes.
- **Reddit** is checked using its JSON endpoint (`/about.json`) instead of the HTML profile page.
- Platforms known to block bots or return false positives (such as LinkedIn, Twitter/X, Instagram, and Facebook) are marked as **MAYBE** instead of CONFIRMED.

This distinction helps reduce false assumptions and makes the results more honest and transparent.

---

## Output and Interaction

After scanning, UniProfile displays the results in a structured table showing:

- Site name  
- Profile URL  
- HTTP status code  
- Detection status (CONFIRMED, MAYBE, NOT FOUND, BLOCKED)

Color-coded output improves readability:
- Green for confirmed profiles
- Yellow for uncertain or blocked results
- Red for not found or errors

The user can then interact with the results using simple commands:

- Enter a site ID number to open that profile in the default browser
- `all` to open all found profiles
- `save` to export results to a `results.txt` file
- `exit` to quit the program

---

## File Structure

This project consists primarily of a single source file:

- **`src/main_improved.c`**  
  Contains the complete implementation, including:
  - Username sanitization
  - HTTP status checking
  - Platform definitions
  - Progress bar rendering
  - Result classification
  - Browser integration
  - File saving logic

The program uses standard C libraries and relies on `curl`, which is commonly available on Linux, macOS, and Windows systems.

---

## Design Decisions

Several deliberate design choices were made during development:

- **Language Choice (C):**  
  C was chosen to keep the tool fast, portable, and dependency-free, unlike many Python-based OSINT tools that rely on large libraries.

- **Security Focus:**  
  Input sanitization was prioritized to prevent shell injection vulnerabilities when using `popen()` and `system()` calls.

- **Minimal Dependencies:**  
  Instead of using third-party libraries, the tool relies on system-native `curl` to maximize portability.

- **Honest Results:**  
  Rather than falsely claiming all detected profiles are valid, the tool differentiates between CONFIRMED and MAYBE results based on platform behavior.

---

## Limitations

While UniProfile is effective, it has some limitations:

- Some platforms aggressively block automated requests, which may lead to false negatives.
- The tool does not currently use concurrency; requests are processed sequentially.
- It checks for username existence only and does not collect additional profile data.

These limitations are intentional to keep the project simple, secure, and aligned with CS50’s learning objectives.

---

## Future Improvements

Potential future enhancements include:

- Multithreading for faster scans
- Exporting results in JSON or CSV format
- Adding more platforms
- Optional proxy or Tor support
- Rate-limit handling and retry logic

---

## Conclusion

UniProfile is a practical, security-conscious OSINT tool that demonstrates systems programming concepts, safe input handling, networking fundamentals, and clean CLI design. This project represents my growth throughout CS50x and my interest in building real-world developer and security tools.

