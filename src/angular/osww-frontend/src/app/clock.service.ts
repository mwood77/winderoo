import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root',
})
export class ClockService {
  private intervalId: any;

  /**
   * Starts a clock that ticks from a given epoch (in seconds).
   * @param startEpoch - The starting epoch in seconds.
   * @param callback - A function that will be called with the current time in seconds.
   */
  startClock(startEpoch: number, callback: (currentTime: number) => void): void {
    let currentEpoch = startEpoch;

    this.intervalId = setInterval(() => {
      currentEpoch++; // Increment the epoch by 1 second
      callback(currentEpoch); // Call the callback function with the updated time
    }, 1000); // Update every second
  }

  /**
   * Stops the clock.
   */
  stopClock(): void {
    if (this.intervalId) {
      clearInterval(this.intervalId);
      this.intervalId = null;
    }
  }
}
