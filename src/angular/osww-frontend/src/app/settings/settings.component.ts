import { Component, OnInit } from '@angular/core';
import { ApiService, Update } from '../api.service';

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
  
  upload = {
    activityState: '',
    statusMessage: '',
    disabled: false,
    direction: '',
    rpd: 0
  }

  watchWindingParametersURL = 'https://watch-winder.store/watch-winding-table/';

  constructor(private apiService: ApiService) { }

  ngOnInit(): void {
    this.upload.statusMessage = 'Save Settings'

    this.apiService.getStatus().subscribe((data) => {
      this.upload.activityState = data.status;
      this.upload.rpd = data.rotationsPerDay;
      this.upload.direction = data.direction;
    })
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
        return 'no-status';
    }
  }

  setRotationsPerDay(rpd: any) {
    this.upload.rpd = rpd.value
  }

  getReadableDirectionOfRotation(direction: string): string {
    switch (direction) {
      case 'CW':
        return 'Clockwise';
      case 'CCW':
        return 'Counter-clockwise';
      case 'BOTH':
        return 'Clockwise & Counter-Clockwise';
      default:
        return 'Clockwise';
    }
  }

  openSite(URL: string) {
    window.open(URL, '_blank');
  }

  uploadSettings(actionToDo?: string) {
    this.upload.statusMessage = 'Upload in progress';
    this.upload.disabled = true;

    const body: Update = {
      action: actionToDo ? actionToDo : 'START',
      rotationDirection: this.upload.direction,
      tpd: this.upload.rpd,
    }

    this.apiService.updateState(body).subscribe((response) => {
      if (response.status == 204) {
        this.apiService.getStatus().subscribe((data) => {
          this.upload.activityState = data.status;
          this.upload.rpd = data.rotationsPerDay;
          this.upload.direction = data.direction;
        })
      }
      this.upload.disabled = false;
      this.upload.statusMessage = 'Save Settings';
    });
  }

  beginProgramming() {
    this.uploadSettings('START');
  }

  stopProgramming() {
    this.uploadSettings('STOP');
  }

}
