const WEEKDAYS = [
    { v: 1, l: "Mon" }, { v: 2, l: "Tue" }, { v: 3, l: "Wed" }, { v: 4, l: "Thu" },
    { v: 5, l: "Fri" }, { v: 6, l: "Sat" }, { v: 0, l: "Sun" },
];

const FREQUENCIES = [
    { key: "minute", label: "Minutes" },
    { key: "hour", label: "Hourly" },
    { key: "day", label: "Daily" },
    { key: "week", label: "Weekly" },
    { key: "month", label: "Monthly" },
];

export function registerCronBuilder() {
    Alpine.data("cronBuilder", (rootEl) => ({
        rootEl,
        frequencies: FREQUENCIES,
        weekdayItems: WEEKDAYS,
        freq: "minute",
        everyMin: 5,
        everyHour: 1,
        minuteOfHour: 0,
        time: "09:00",
        weekdays: [],
        monthdays: "1",
        init() {
            const parent = this.parent();
            const current = parent ? parent.currentValue() : "";
            if (current && current.trim()) {
                if (!this.parseExpression(current)) {
                    if (parent) parent.setMode("manual");
                    return;
                }
            }
            this.emit();
        },
        parent() {
            const node = this.rootEl.closest("[x-data*='cronInput']");
            return node ? Alpine.$data(node) : null;
        },
        clockTime() {
            const [h, m] = (this.time || "00:00").split(":").map(x => parseInt(x, 10) || 0);
            return { h, m };
        },
        setTime(h, m) {
            this.time = String(h).padStart(2, "0") + ":" + String(m).padStart(2, "0");
        },
        expression() {
            const { h, m } = this.clockTime();
            switch (this.freq) {
                case "minute": {
                    const n = Math.max(1, parseInt(this.everyMin, 10) || 1);
                    return n === 1 ? "* * * * *" : `*/${n} * * * *`;
                }
                case "hour": {
                    const n = Math.max(1, parseInt(this.everyHour, 10) || 1);
                    const mm = Math.min(59, Math.max(0, parseInt(this.minuteOfHour, 10) || 0));
                    return n === 1 ? `${mm} * * * *` : `${mm} */${n} * * *`;
                }
                case "day":
                    return `${m} ${h} * * *`;
                case "week": {
                    const days = this.weekdays.length ? [...this.weekdays].sort((a, b) => a - b).join(",") : "*";
                    return `${m} ${h} * * ${days}`;
                }
                case "month": {
                    const d = (this.monthdays || "").trim() || "1";
                    return `${m} ${h} ${d} * *`;
                }
            }
            return "* * * * *";
        },
        parseExpression(expr) {
            const t = expr.trim().split(/\s+/);
            if (t.length !== 5) return false;
            const [mi, hr, dom, mon, dow] = t;
            const isNum = s => /^\d+$/.test(s);
            if (mon === "*" && dom === "*" && dow === "*") {
                if (hr === "*") {
                    if (mi === "*") { this.freq = "minute"; this.everyMin = 1; return true; }
                    const sm = mi.match(/^\*\/(\d+)$/);
                    if (sm) { this.freq = "minute"; this.everyMin = +sm[1]; return true; }
                    if (isNum(mi)) { this.freq = "hour"; this.everyHour = 1; this.minuteOfHour = +mi; return true; }
                    return false;
                }
                const sh = hr.match(/^\*\/(\d+)$/);
                if (sh && isNum(mi)) { this.freq = "hour"; this.everyHour = +sh[1]; this.minuteOfHour = +mi; return true; }
                if (isNum(hr) && isNum(mi)) { this.freq = "day"; this.setTime(+hr, +mi); return true; }
                return false;
            }
            if (dom === "*" && mon === "*" && dow !== "*" && isNum(hr) && isNum(mi)) {
                const days = dow.split(",");
                if (days.every(isNum)) { this.freq = "week"; this.weekdays = days.map(Number); this.setTime(+hr, +mi); return true; }
                return false;
            }
            if (dow === "*" && mon === "*" && dom !== "*" && isNum(hr) && isNum(mi)) {
                this.freq = "month"; this.monthdays = dom; this.setTime(+hr, +mi); return true;
            }
            return false;
        },
        emit() {
            const parent = this.parent();
            if (parent) parent.setExpression(this.expression());
        },
        setFreq(f) { this.freq = f; this.emit(); },
        toggleWeekday(v) {
            const i = this.weekdays.indexOf(v);
            if (i >= 0) this.weekdays.splice(i, 1);
            else this.weekdays.push(v);
            this.emit();
        },
    }));
}
