export class LogService {
    constructor(api) { this.api = api; }
    list() { return this.api.get("/api/logs"); }
    get(id) { return this.api.get(`/api/logs/${id}`); }
    deleteMany(ids) { return this.api.delete("/api/logs", { ids }); }
}
