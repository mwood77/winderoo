# Winderoo Frontend

## Setup

1. [Get NodeJS](https://nodejs.org/en/download/)
1. [Get the Angular CLI](https://angular.io/guide/setup-local#install-the-angular-cli)
1. Navigate to the following directory, `src/angular/osww-frontend/`, in your terminal / shell.
1. Run `npm i`
1. Proceed to **Development server**

## Development server

1. Run `npm run start` from this location (`src/angular/osww-frontend/`)
1. Navigate to `http://localhost:4200/` in your browser. You'll see the application.
    - ⚠️ You'll need Winderoo's API running (on hardware, on your network) to interact with. 
    - See [this README](../../../docs/install-software.md) for instructions on how to flash the API to your hardware.
1. Happy hacking 

## Build & Deploy

1. Run `npm run build-arduino` to build the project. 
    - This scrip generates build artifacts in the `dist/` directory.
    - These artifacts will then be compressed with `gzip` and copied to the `../../../data/` directory.
1. [Flash the file system image to your hardware - Step 6](../../../docs/install-software.md); PlatformIO > **Upload Filesystem Image**. 
