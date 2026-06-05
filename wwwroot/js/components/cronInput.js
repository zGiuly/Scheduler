export function registerCronInput(cronParser) {
    Alpine.data("cronInput", () => ({
        mode: "builder",
        feedback: { icon: "○", text: "Enter a cron expression", valid: false, invalid: false },
        $input: null,
        bind(inputEl) {
            this.$input = inputEl;
            this.update(inputEl.value);
            inputEl.addEventListener("input", () => this.update(inputEl.value));
        },
        setMode(mode) { this.mode = mode; },
        currentValue() { return this.$input ? this.$input.value : ""; },
        update(value) {
            const result = cronParser.parse(value);
            if (!value.trim()) {
                this.feedback = { icon: "○", text: "Enter a cron expression", valid: false, invalid: false };
            } else if (result.valid) {
                this.feedback = { icon: "✓", text: result.description, valid: true, invalid: false };
            } else {
                this.feedback = { icon: "✕", text: result.message, valid: false, invalid: true };
            }
        },
        setExpression(expr) {
            if (!this.$input) return;
            this.$input.value = expr;
            this.$input.dispatchEvent(new Event("input", { bubbles: true }));
            this.update(expr);
        },
    }));
}
