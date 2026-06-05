function formatDate(value) {
    if (!value) return "—";
    const d = new Date(value);
    return isNaN(d.getTime()) ? value : d.toLocaleString();
}

export function registerLogsView(logService) {
    Alpine.data("logsView", () => ({
        allLogs: [],
        loading: true,
        filters: { search: "", project: "", type: "" },
        selected: [],
        async init() {
            await this.refresh();
            window.addEventListener("logs:refresh", () => this.refresh());
        },
        async refresh() {
            this.loading = true;
            try {
                this.allLogs = await logService.list();
                this.selected = this.selected.filter(id => this.allLogs.some(l => l.id === id));
            } catch (e) { Alpine.store("toast").error(e.message); }
            finally { this.loading = false; }
        },
        get projectOptions() {
            return [...new Set(this.allLogs.map(l => l.project))].sort();
        },
        get filtered() {
            const { search, project, type } = this.filters;
            const q = search.toLowerCase();
            return this.allLogs.filter(l => {
                if (project && l.project !== project) return false;
                if (type && l.type !== type) return false;
                if (q && !`${l.name} ${l.project} ${l.description || ""}`.toLowerCase().includes(q)) return false;
                return true;
            });
        },
        get allFilteredSelected() {
            const f = this.filtered;
            return f.length > 0 && f.every(l => this.selected.includes(l.id));
        },
        get someFilteredSelected() {
            const f = this.filtered;
            return f.some(l => this.selected.includes(l.id)) && !this.allFilteredSelected;
        },
        isSelected(id) { return this.selected.includes(id); },
        toggleSelect(id) {
            const i = this.selected.indexOf(id);
            if (i >= 0) this.selected.splice(i, 1);
            else this.selected.push(id);
        },
        toggleSelectAll() {
            if (this.allFilteredSelected) {
                const filteredIds = new Set(this.filtered.map(l => l.id));
                this.selected = this.selected.filter(id => !filteredIds.has(id));
            } else {
                const all = new Set(this.selected);
                this.filtered.forEach(l => all.add(l.id));
                this.selected = [...all];
            }
        },
        clearFilters() { this.filters = { search: "", project: "", type: "" }; },
        clearSelection() { this.selected = []; },
        formatDate,
        async deleteLog(log) {
            if (!confirm(`Delete execution log #${log.id}? This cannot be undone.`)) return;
            try {
                await logService.deleteMany([log.id]);
                this.allLogs = this.allLogs.filter(l => l.id !== log.id);
                this.selected = this.selected.filter(id => id !== log.id);
                Alpine.store("toast").success(`Log #${log.id} deleted`);
            } catch (e) { Alpine.store("toast").error(e.message); }
        },
        async deleteSelected() {
            if (!this.selected.length) return;
            const count = this.selected.length;
            if (!confirm(`Delete ${count} execution log${count > 1 ? "s" : ""}? This cannot be undone.`)) return;
            const ids = [...this.selected];
            try {
                await logService.deleteMany(ids);
                this.allLogs = this.allLogs.filter(l => !ids.includes(l.id));
                this.selected = [];
                Alpine.store("toast").success(`${count} log${count > 1 ? "s" : ""} deleted`);
            } catch (e) { Alpine.store("toast").error(e.message); }
        },
    }));
}
