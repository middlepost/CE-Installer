# Chrome Extension Installer

A professional tool for installing and managing Chrome extensions programmatically. This tool provides both a CLI interface and a GUI mode for installing Chrome extensions from various sources.

## Features

- Command-line interface for automated installation
- GUI mode for interactive installation
- Support for local and remote extension sources
- Automatic elevation handling for admin privileges
- System compatibility checks
- Secure download and installation process

## Prerequisites

- Windows operating system
- Google Chrome browser installed
- 7-Zip for handling extension packages
- Visual Studio 2019 or later for building

## Building

1. Open `ChromeExtension.sln` in Visual Studio
2. Select your target configuration (Debug/Release)
3. Build the solution

## Usage

### CLI Mode

```bash
ChromeExtension.exe [options]
  -i, --install <extension-id>    Install extension by Chrome Web Store ID
  -f, --file <path>              Install extension from local file
  -u, --url <url>                Install extension from URL
  -q, --quiet                    Run in quiet mode (no GUI)
  -h, --help                     Show help message
```

### GUI Mode

Simply run the executable without arguments to launch in GUI mode.

## Security

- All downloads are verified for integrity
- Admin privileges are requested only when necessary
- Virtual machine detection for security
- Memory and system requirement validation

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the terms of the license included in the repository.

## Disclaimer

This tool should be used in compliance with Chrome Web Store policies and terms of service. The authors are not responsible for any misuse of this tool. 