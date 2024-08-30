# OS Build Utility

`os-build-utility` is a command-line tool developed by the [DoorOS Project](https://github.com/DoorOS-Project) designed to simplify the building and running of operating systems. This tool automates compilation, and execution tasks, enhancing efficiency in OS development.

## Installation

1. **Download the Pre-built Binary**:
   Obtain the latest release from the [Releases page](https://github.com/DoorOS-Project/os-build-utility/releases).

2. **Extract and Install**:
   Follow the installation instructions provided with the release assets.

## Usage

The `os-build-utility` command supports the following usage format:

```sh
os-build-utility [[--configure | -c] | [[--build | -b] [--run | -r]]]
```

### Options

- **Configure the Tool**: Set up build and run commands interactively.
  ```sh
  os-build-utility [--configure | -c]
  ```

- **Build the OS**: Compile the operating system.
  ```sh
  os-build-utility [--build | -b]
  ```

- **Run the OS**: Execute the operating system.
  ```sh
  os-build-utility [--run | -r]
  ```

- **Build and Run the OS**: Perform both build and run operations.
  ```sh
  os-build-utility [--build --run | -b -r | -br]
  ```

## Build Instructions

To build the tool from source:

1. **Clone the Repository**:
   ```sh
   git clone https://github.com/DoorOS-Project/os-build-utility.git
   ```

2. **Navigate to the Project Directory**:
   ```sh
   cd os-build-utility
   ```

3. **Build the Tool**:
   ```sh
   make
   ```

## License

`os-build-utility` is licensed under the [GNU General Public License v2.0](https://opensource.org/licenses/GPL-2.0). Please refer to the [`LICENSE`](https://github.com/DoorOS-Project/os-build-utility/tree/main/LICENSE) file for detailed licensing information.

## Contributing

We welcome contributions to `os-build-utility`. To suggest improvements, report issues, or submit code changes, please use the following links:

- [Open an issue](https://github.com/DoorOS-Project/os-build-utility/issues)
- [Submit a pull request](https://github.com/DoorOS-Project/os-build-utility/pulls)
