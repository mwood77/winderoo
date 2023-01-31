import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-settings',
  templateUrl: './settings.component.html',
  styleUrls: ['./settings.component.scss']
})
export class SettingsComponent implements OnInit {

  /**
   * Winder status:
   * -- Winding
   * -- Not Configured
   * -- Paused
   */
  direction = '';
  rpd = 0

  upload = {
    activityState: '',
    statusMessage: '',
    disabled: false,
  }

  watchWindingParametersURL = 'https://watch-winder.store/watch-winding-table/';

  constructor() { }

  ngOnInit(): void {
    this.upload.activityState = 'Paused';
    this.direction = 'Clockwise';
    this.rpd = 460;

    this.upload.statusMessage = 'Save Settings'
  }

  getColour(status: string): string {
    switch (status) {
      case 'Winding':
        return 'green';
      case 'Stopped':
        return 'red';
      case 'Paused':
        return 'yellow';
      default:
        return 'yellow';
    }
  }

  setRotationsPerDay(rpd: any) {
    this.rpd = rpd.value
  }

  setDirectionOfRotation(direction: string): string {
    switch (direction) {
      case 'clockwise':
        return 'cw';
      case 'counter-clockwise':
        return 'ccw';
      case 'clockwise & counter-clockwise':
        return 'both';
      default:
        return 'cw';
    }
  }

  openSite(URL: string) {
    window.open(URL, '_blank');
  }

  uploadSettings() {
    this.upload.statusMessage = 'Upload in progress';
    this.upload.disabled = true;
    setTimeout(() => {
      this.upload.disabled = false;
      this.upload.statusMessage = 'Save Settings'
    }, 2000)
  }

  beginProgramming() {
    this.uploadSettings();
    this.upload.activityState = 'Winding';
  }

  stopProgramming() {
    this.uploadSettings();
    this.upload.activityState = 'Stopped';
  }

}
