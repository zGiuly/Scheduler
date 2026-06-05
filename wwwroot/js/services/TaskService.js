export class TaskService {
    constructor(api) { this.api = api; }
    list() { return this.api.get("/api/tasks"); }
    create(task) { return this.api.post("/api/tasks", task); }
    update(id, task) { return this.api.put(`/api/tasks/${id}`, task); }
    trigger(id) { return this.api.post(`/api/tasks/${id}/trigger`); }
}
