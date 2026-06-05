const EMPTY = {
    project: "",
    name: "",
    description: "",
    command: "",
    cron: "",
    type: "async",
    oneshot: false,
    disabled: false,
};

export function registerTaskForm(taskService) {
    Alpine.data("taskForm", () => ({
        data: { ...EMPTY },
        submitting: false,
        init() {
            const m = Alpine.store("modal");
            this.data = m.type === "taskEdit" && m.payload ? this.fromTask(m.payload) : { ...EMPTY };
        },
        fromTask(t) {
            return {
                project: t.project || "",
                name: t.name || "",
                description: t.description || "",
                command: t.command || "",
                cron: t.cron || "",
                type: t.type || "async",
                oneshot: !!t.oneshot,
                disabled: !!t.disabled,
            };
        },
        async submit() {
            if (!this.data.cron.trim()) {
                Alpine.store("toast").error("Cron expression is required");
                return;
            }
            this.submitting = true;
            try {
                const modal = Alpine.store("modal");
                if (modal.type === "taskEdit") await taskService.update(modal.payload.id, this.data);
                else await taskService.create(this.data);
                Alpine.store("toast").success(modal.type === "taskEdit" ? "Task updated" : "Task created");
                modal.close();
                window.dispatchEvent(new CustomEvent("tasks:refresh"));
            } catch (e) {
                Alpine.store("toast").error(e.message);
            } finally {
                this.submitting = false;
            }
        },
    }));
}
