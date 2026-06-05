const ROUTES = [
    { pattern: /^\/tasks$/, build: () => ({ name: "tasks", title: "Tasks", subtitle: "Manage scheduled tasks" }) },
    { pattern: /^\/logs$/, build: () => ({ name: "logs", title: "Execution Logs", subtitle: "Inspect task run history" }) },
    { pattern: /^\/logs\/(\d+)$/, build: m => ({ name: "logDetail", params: { id: m[1] }, title: `Execution Log #${m[1]}`, subtitle: "Full execution details" }) },
];

export class HashRouter {
    constructor(onChange) { this.onChange = onChange; }
    start() {
        window.addEventListener("hashchange", () => this.resolve());
        this.resolve();
    }
    navigate(path) {
        if (location.hash.slice(1) === path) this.resolve();
        else location.hash = path;
    }
    resolve() {
        const path = location.hash.slice(1) || "/tasks";
        for (const route of ROUTES) {
            const m = path.match(route.pattern);
            if (m) { this.onChange(route.build(m)); return; }
        }
        this.navigate("/tasks");
    }
}
