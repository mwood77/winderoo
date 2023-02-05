# OSWW-Frontend (Winderoo)

## Setup

1. [Get NodeJS](https://nodejs.org/en/download/)
1. [Get the Angular CLI](https://angular.io/guide/setup-local#install-the-angular-cli)
1. Navigate to the following directory, `src/angular/osww-frontend/`, in your terminal / shell.
1. Run `npm i`
1. Proceed to **Development server**

## Development server

1. Run `npm run serve` from this location (`src/angular/osww-frontend/`) & navigate to `http://localhost:4200/` in your browser. You'll see the application.
    - ⚠️ You'll need Winderoo's API running (on hardware, on your network) to interact with. 
    - See [this README](../../../docs/install-software.md) for instructions on how to run the API on hardware.
1. Happy hacking 

## Build

1. Run `npm run build-arduino` to build the project. 
1. The build artifacts will be generated in the `dist/` directory.
1. The build artifacts will then be `gzip`'d and copied to `../../../data/` directory.
