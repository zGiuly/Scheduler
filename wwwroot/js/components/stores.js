export function registerStores(router) {
    Alpine.store("app", {
        route: { name: "tasks", title: "Tasks", subtitle: "Manage scheduled tasks", params: {} },
        connected: false,
        setRoute(r) { this.route = { params: {}, ...r }; },
        setConnected(c) { this.connected = c; },
        navigate(path) { router.navigate(path); },
    });

    Alpine.store("modal", {
        open: false,
        type: null,
        payload: null,
        show(type, payload = null) { this.type = type; this.payload = payload; this.open = true; },
        close() { this.open = false; this.type = null; this.payload = null; },
    });

    Alpine.store("toast", {
        items: [],
        nextId: 1,
        show(message, kind = "info") {
            const id = this.nextId++;
            this.items.push({ id, message, kind });
            setTimeout(() => { this.items = this.items.filter(t => t.id !== id); }, 3200);
        },
        success(m) { this.show(m, "success"); },
        error(m) { this.show(m, "error"); },
    });
}
