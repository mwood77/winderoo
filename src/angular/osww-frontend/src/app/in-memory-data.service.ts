import { InMemoryDbService } from 'angular-in-memory-web-api';
import { Injectable } from '@angular/core';

// This file is for local development only. You can delete or move it to another branch.
@Injectable({
  providedIn: 'root',
})
export class InMemoryDataService implements InMemoryDbService {
  createDb() {
    // Mock data for /api/status endpoint
    const status = {
      status: 'Stopped',
      rotationsPerDay: 650,
      direction: 'CW',
      hour: '08',
      minutes: '30',
      startTimeEpoch: 0,
      estimatedRoutineFinishEpoch: 0,
      timerEnabled: 1,
      screenSleep: false,
      screenEquipped: true,
      currentTimeEpoch: 1710000000,
      customDurationInSecondsToCompleteOneRevolution: 8,
      gmtOffset: 0,
      dst: false,
      customWindDuration: 180,
      customWindPauseDuration: 15,
      winderEnabled: 1
    };
    return { status };
  }
} 