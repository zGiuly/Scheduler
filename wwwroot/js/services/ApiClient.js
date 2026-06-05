export class ApiClient {
    constructor(baseUrl) { this.baseUrl = baseUrl; }
    async request(path, options = {}) {
        const res = await fetch(`${this.baseUrl}${path}`, {
            method: options.method || "GET",
            headers: { "Content-Type": "application/json" },
            body: options.body,
        });
        const text = await res.text();
        const data = text ? JSON.parse(text) : null;
        if (!res.ok) throw new Error(data?.error || `Request failed (${res.status})`);
        return data;
    }
    get(path) { return this.request(path); }
    post(path, body) { return this.request(path, { method: "POST", body: JSON.stringify(body || {}) }); }
    put(path, body) { return this.request(path, { method: "PUT", body: JSON.stringify(body || {}) }); }
    delete(path, body) {
        return this.request(path, {
            method: "DELETE",
            body: body !== undefined ? JSON.stringify(body) : undefined,
        });
    }
}
