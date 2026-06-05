export class ConnectionMonitor {
    constructor(api, onChange, intervalMs = 8000) {
        this.api = api;
        this.onChange = onChange;
        this.intervalMs = intervalMs;
    }
    start() {
        this.check();
        setInterval(() => this.check(), this.intervalMs);
    }
    async check() {
        try { await this.api.get("/api/tasks"); this.onChange(true); }
        catch { this.onChange(false); }
    }
}
