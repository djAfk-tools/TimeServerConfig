# TimeServerConfig
A C++ MFC application for configuring and managing NTP (Network Time Protocol) server settings designed for Windows Server 2016–2022. This tool provides a user-friendly interface to modify time synchronization settings, test NTP servers, and manage registry backups, requires administrative privileges due to modifying HKLM registry.

# License:
This project is licensed under the MIT License with a No-Sale Restriction. You can use, modify, and include it in other works (even commercial ones), but you may not sell or distribute TimeServerConfig as a standalone commercial product. See the [LICENSE](LICENSE.txt) file for details.
Copyright (c) 2025 djAfk

# Download Binary/EXE:


# Features:
Reads current registry settings to populate window.
Configure time synchronization type: Domain (NT5DS), Manual (NTP), or NoSync.
Manage up to three NTP servers with customizable server addresses and flags (e.g., 0x9, 0x1, 0x8).
Real-time display of NTP server format (e.g., time.windows.com,0x9).
Test NTP servers using w32tm /stripchart.
Sync system time with w32tm /resync.
Backup and restore time settings via registry export/import.
Refresh settings from the registry.
OS version checking (Windows Server 2016–2022), will still attempt to run on other OS.
Group Policy override warnings.
Requires administrative privileges (UAC elevation).

# Prerequisites:
Intended OS Windows Server 2016–2022.
Visual Studio 2022 with MFC support for compiling the application (binary available).
Administrative privileges to run the application.

# Setup Instructions:
(Binary availalbe for download if you do not want to compile)
Clone the repository:

git clone https://github.com/djAfk-tools/TimeServerConfig.git

Open TimeServerConfig.sln in Visual Studio 2022.
Configure project settings:General: Set Character Set to Multi-Byte, MFC to Static Library.
C/C++ > Precompiled Headers: Use /Yu, File = pch.h.
Linker > Input: Add advapi32.lib;shell32.lib.
Linker > Manifest File: Set UAC Execution Level to requireAdministrator, Additional Manifest Files to $(ProjectDir)timeserverconfig.exe.manifest.
(may need to remove other included manifests)

Build the solution (Ctrl+Shift+B).
Run as administrator to configure NTP settings.

Usage:Launch the application with administrative privileges.
Select synchronization type from the dropdown.
For Manual (NTP), enter up to three NTP server addresses and select flags.
Click "Test" to verify NTP servers, "Sync Now" to synchronize time, "Apply" to save settings, "Save/Backup" to export registry, or "Restore/Import" to import settings.

Registry Interaction:Reads/writes to HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\W32Time\Parameters (NtpServer, Type).
[Future release:] Reads/writes advanced settings in HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\W32Time\TimeProviders\NtpClient (MinPollInterval, MaxPollInterval, SpecialPollInterval).
