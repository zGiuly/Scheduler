export class CronParser {
    constructor(library) { this.lib = library; }
    parse(expression) {
        const expr = (expression || "").trim();
        if (!expr) return { valid: false, message: "Cron expression is required" };
        try {
            const description = this.lib.toString(expr, { use24HourTimeFormat: true });
            return { valid: true, description };
        } catch (e) {
            return { valid: false, message: typeof e === "string" ? e : (e.message || "Invalid cron expression") };
        }
    }
}
