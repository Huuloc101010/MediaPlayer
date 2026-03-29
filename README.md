<img width="1275" height="964" alt="image" src="https://github.com/user-attachments/assets/eff8db41-2b4b-49b7-acba-a95257977670" />

# MediaPlayer

# C++ Media Player (FFmpeg + SDL2)

## Overview

This project is a simple media player written in **C++20**, designed with a focus on:

- Modern C++ practices
- Thread safety
- Modular architecture

The application uses:

- **Smart pointers** for safe memory management  
- **std::mutex** and **std::condition_variable** for thread synchronization  
- **FFmpeg (libavformat, libavcodec, libavutil, etc.)** for media processing  
- **SDL2** for:
  - Event handling (keyboard, mouse)
  - Video rendering
  - Audio playback  

---

## Architecture

The project follows a modular design with clear separation of responsibilities between components.

---

## Class Responsibilities

### Audio

- **AudioDecoder**
  - Responsible for decoding audio streams

- **AudioDevice**
  - Handles low-level audio playback using SDL2

- **AudioOutput**
  - Configures audio output
  - Maintains audio clock for synchronization

---

### Video

- **VideoDecoder**
  - Responsible for decoding video streams

- **VideoOutput**
  - Handles processed video frames before rendering

- **VideoRenderer**
  - Performs actual rendering
  - Only renders based on instructions from `View`

---

### Core Processing

- **Demuxer**
  - Splits media into audio and video streams

- **Decoder**
  - Base class containing shared logic for audio/video decoders

- **Output**
  - Base class for `AudioOutput` and `VideoOutput`

---

### Control & Interaction

- **Controller**
  - Captures user input (keyboard, mouse)

- **ControlFunction**
  - Defines interfaces for player control actions (play, pause, next, etc.)

---

### Core System

- **Player**
  - Central class
  - Coordinates all components

- **PlayList**
  - Manages media list
  - Supports switching between media (next/previous)

- **Mediator**
  - Handles communication between components (decoupling)

---

### UI & Rendering

- **View**
  - Distributes data to output components
  - Manages layout and UI element sizing

- **Window**
  - Handles SDL window creation and management

---

### Utilities

- **SafeQueue**
  - Thread-safe blocking queue
  - Uses `mutex` and `condition_variable`
  - Ensures safe communication between threads

- **Log**
  - Logging utility

- **Define**
  - Common definitions and macros

---

## Project Structure

```text
MediaPlayer/
├── inc/ # Header files
├── src/ # Source files
├── build/ # Object files
├── sample/ # Sample media files
├── icon/ # UI assets
├── Makefile
├── prepare_env.sh
├── README.md
└── player # Output executable


---

## Features

- Play audio and video using FFmpeg
- Multi-threaded decoding and rendering
- Basic media controls (play, next, previous)
- Playlist management
- SDL2-based rendering and audio output

---

## Build & Run


$ ./prepare_env.sh
make
$ ./player sample

```

