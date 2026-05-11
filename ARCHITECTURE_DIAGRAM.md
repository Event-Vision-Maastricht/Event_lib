# Event Library Architecture Diagram

## Centralized Header Flow

```
                    ┌─────────────────────────────────┐
                    │    SensorMetadata (core)        │
                    │  ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔  │
                    │  • width, height                │
                    │  • date, time                   │
                    │  • version, event_type          │
                    │  • is_valid() method            │
                    │  • virtual destructor           │
                    └────────────┬────────────────────┘
                                 │
                    ┌────────────┼────────────┐
                    │            │            │
                    ▼            ▼            ▼
         ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐
         │  event_parser.   │  │   DatParser.     │  │  visualize.  │
         │       hpp        │  │       hpp        │  │      hpp     │
         │ ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ │  │ ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ │  │ ▔▔▔▔▔▔▔▔▔▔▔ │
         │ FileHeader*     │  │ DatFileHeader*   │  │ SensorMetadata│
         │ (now alias)     │  │ (now alias)      │  │ (direct use)  │
         │                 │  │                  │  │               │
         │ EventParser     │  │ DatParser        │  │ visualize     │
         │ interface       │  │ implementation   │  │ class         │
         └────────┬────────┘  └────────┬────────┘  └───────┬───────┘
                  │                    │                   │
                  └────────────────────┼───────────────────┘
                                       │
                                       ▼
                          ┌──────────────────────┐
                          │  All Parser/Stream   │
                          │     Instances        │
                          │ ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ │
                          │                      │
                          │ const FileHeader&    │
                          │ header() const;      │
                          │ (returns SensorM.)   │
                          └──────────┬───────────┘
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
                    ▼                ▼                ▼
         ┌──────────────────┐  ┌──────────────┐  ┌──────────────┐
         │ DatasetEventStream│  │  Frame       │  │ FrameQueue   │
         │ ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔│  │ ▔▔▔▔▔▔▔▔▔▔▔ │  │ ▔▔▔▔▔▔▔▔▔ │
         │ • Get metadata   │  │ • Uses w,h   │  │ • Uses w,h   │
         │   from parser    │  │   from meta  │  │   from meta  │
         │ • Pass to        │  │ • Can init   │  │ • Can init   │
         │   visualization  │  │   from meta  │  │   from meta  │
         └──────────────────┘  └──────────────┘  └──────────────┘
```

## Module Dependency Graph

```
                    ┌─────────────────────┐
                    │  sensor_metadata.   │
                    │    hpp (CORE)       │
                    └──────────┬──────────┘
                               │
                    ┌──────────┼──────────┐
                    │          │          │
                    ▼          ▼          ▼
            ┌───────────┐ ┌───────────┐ ┌──────────────┐
            │ event_    │ │ DatParser │ │  visualize   │
            │ parser    │ │    .hpp   │ │    .hpp      │
            │ .hpp      │ │           │ │              │
            │ (CORE)    │ │ (PARSER)  │ │(PROCESSING)  │
            └─────┬─────┘ └─────┬─────┘ └──────┬───────┘
                  │             │              │
                  ▼             ▼              ▼
            ┌─────────────────────────────────────────┐
            │      Used by Streams/Processing         │
            │  DatasetEventStream, Frame, FrameQueue  │
            └─────────────────────────────────────────┘
```

## Backward Compatibility Aliases

```
Old Code             │   New Reality
─────────────────────┼──────────────────────────
FileHeader           │   using FileHeader = SensorMetadata
    ↓                │   ↓
  Used in            │   All point to
  event_parser.hpp   │   centralized SensorMetadata
─────────────────────┼──────────────────────────
DatFileHeader        │   using DatFileHeader = SensorMetadata
    ↓                │   ↓
  Used in            │   All point to
  DatParser.hpp      │   centralized SensorMetadata
─────────────────────┼──────────────────────────
SensorMetadata       │   struct SensorMetadata
    ↓                │   ↓
  Used in            │   Now centralized
  visualize.hpp      │   in sensor_metadata.hpp
```

## Information Flow: Reading a DAT File

```
User Code
    │
    ├─→ Opens: DatasetEventStream("file.dat")
    │
    ├─→ Creates: EventParserFactory::create_parser()
    │   │
    │   └─→ Returns: std::unique_ptr<DatParser>
    │
    ├─→ Calls: parser->open("file.dat")
    │   │
    │   └─→ Reads header → Populates SensorMetadata
    │
    ├─→ Gets: const FileHeader& = parser->header()
    │   │
    │   └─→ Returns: const SensorMetadata&
    │
    ├─→ Accesses: metadata.width, metadata.height, etc.
    │
    └─→ Uses for: Frame(w, h), visualize init, etc.

    All use SensorMetadata from sensor_metadata.hpp!
```

## Data Structure in Memory

```
Before Refactoring (Duplicated):
┌─────────────────────────────────────────────────────┐
│  event_parser.hpp: FileHeader {                     │
│    int width, height                                │
│    std::string date, time, version, event_type      │
│  }                                                  │
├─────────────────────────────────────────────────────┤
│  DatParser.hpp: DatFileHeader : public FileHeader { │
│    int width, height  // DUPLICATE!                 │
│    std::string date, time, version, event_type      │
│  }                                                  │
├─────────────────────────────────────────────────────┤
│  visualize.hpp: SensorMetadata {                    │
│    int width, height  // DUPLICATE!                 │
│    std::string date, time, version, event_type      │
│  }                                                  │
└─────────────────────────────────────────────────────┘
     Problem: Same data defined 3 times!

After Refactoring (Centralized):
┌─────────────────────────────────────────────────────┐
│  sensor_metadata.hpp: SensorMetadata {              │
│    int width, height                                │
│    std::string date, time, version, event_type      │
│    bool is_valid() const                            │
│    virtual ~SensorMetadata()                        │
│  }                                                  │
├─────────────────────────────────────────────────────┤
│  event_parser.hpp: using FileHeader = SensorMetadata│
│  DatParser.hpp: using DatFileHeader = SensorMetadata│
│  visualize.hpp: uses SensorMetadata directly        │
└─────────────────────────────────────────────────────┘
     Solution: Single definition, multiple aliases!
```

## Complete Module Interactions

```
┌────────────────────────────────────────────────────────────┐
│                   APPLICATION LAYER                        │
│                   (User Code)                              │
└────────────┬───────────────────────────────────────────────┘
             │
             │ Creates
             ▼
┌────────────────────────────────────────────────────────────┐
│                   STREAM LAYER                             │
│         DatasetEventStream                                 │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Owns: std::unique_ptr<EventParser> parser_         │    │
│  │ Has:  const SensorMetadata* fh_ptr_                │    │
│  │ Provides: header() -> const FileHeader&            │    │
│  └────────────────────────────────────────────────────┘    │
└────────────┬───────────────────────────────────────────────┘
             │
             │ Uses
             ▼
┌────────────────────────────────────────────────────────────┐
│                   PARSER LAYER                             │
│  ┌─────────────────┐  ┌──────────────────┐                │
│  │  EventParser    │  │  DatParser       │                │
│  │  (Interface)    │  │  (Implementation)│                │
│  │  ┌─────────────┐│  │  ┌──────────────┐│                │
│  │  │header()      ││  │  │DatFileHeader ││                │
│  │  │returns const ││  │  │(= SensorMeta)││                │
│  │  │FileHeader&   ││  │  └──────────────┘│                │
│  │  │(= SensorMeta)││  │                  │                │
│  │  └─────────────┘│  │                  │                │
│  └─────────────────┘  └──────────────────┘                │
└────────────┬───────────────────────────────────────────────┘
             │
             │ Returns SensorMetadata via header()
             │
    ┌────────┴────────┬───────────────┐
    │                 │               │
    ▼                 ▼               ▼
┌──────────┐  ┌──────────────┐  ┌──────────────┐
│  Frame   │  │  FrameQueue  │  │  visualize   │
│ (width,  │  │ (width,      │  │ (init_meta)  │
│  height) │  │  height)     │  │              │
└──────────┘  └──────────────┘  └──────────────┘

All modules get sensor metadata from the same place!
```

## Include Dependency Tree

```
sensor_metadata.hpp (CORE - no dependencies)
    │
    ├── event_parser.hpp
    │   ├── event_packet.hpp (not shown)
    │   └── Used by: EventParser interface
    │
    ├── DatParser.hpp
    │   ├── event_parser.hpp
    │   └── Used by: DatParser implementation
    │
    └── visualize.hpp
        ├── Frame.hpp
        ├── FrameQueue.hpp
        └── Used by: visualize class

Single central include eliminates duplication!
```

