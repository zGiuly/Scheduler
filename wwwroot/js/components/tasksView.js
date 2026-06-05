export function registerTasksView(taskService) {
    Alpine.data("tasksView", () => ({
        tasks: [],
        loading: true,
        async init() {
            await this.refresh();
            window.addEventListener("tasks:refresh", () => this.refresh());
        },
        async refresh() {
            this.loading = true;
            try { this.tasks = await taskService.list(); }
            catch (e) { Alpine.store("toast").error(e.message); }
            finally { this.loading = false; }
        },
        async trigger(task) {
            try {
                await taskService.trigger(task.id);
                Alpine.store("toast").success(`Triggered "${task.name}"`);
            } catch (e) { Alpine.store("toast").error(e.message); }
        },
    }));
}
