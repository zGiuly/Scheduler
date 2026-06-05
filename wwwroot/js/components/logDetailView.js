function formatDate(value) {
    if (!value) return "—";
    const d = new Date(value);
    return isNaN(d.getTime()) ? value : d.toLocaleString();
}

export function registerLogDetailView(logService) {
    Alpine.data("logDetailView", () => ({
        log: null,
        loading: false,
        error: null,
        formatDate,
        init() {
            this.$watch("$store.app.route", route => {
                if (route.name === "logDetail") this.load(route.params.id);
            });
            window.addEventListener("log:delete-current", () => this.deleteCurrent());
            if (Alpine.store("app").route.name === "logDetail") {
                this.load(Alpine.store("app").route.params.id);
            }
        },
        async load(id) {
            this.loading = true;
            this.log = null;
            this.error = null;
            try { this.log = await logService.get(id); }
            catch (e) { this.error = e.message; Alpine.store("toast").error(e.message); }
            finally { this.loading = false; }
        },
        async deleteCurrent() {
            if (!this.log) return;
            const id = this.log.id;
            if (!confirm(`Delete execution log #${id}? This cannot be undone.`)) return;
            try {
                await logService.deleteMany([id]);
                Alpine.store("toast").success(`Log #${id} deleted`);
                Alpine.store("app").navigate("/logs");
            } catch (e) { Alpine.store("toast").error(e.message); }
        },
    }));
}
