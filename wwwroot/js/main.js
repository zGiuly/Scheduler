import Alpine from "https://cdn.jsdelivr.net/npm/alpinejs@3.14.1/dist/module.esm.js";
import { ApiClient } from "./services/ApiClient.js";
import { TaskService } from "./services/TaskService.js";
import { LogService } from "./services/LogService.js";
import { CronParser } from "./services/CronParser.js";
import { ConnectionMonitor } from "./services/ConnectionMonitor.js";
import { HashRouter } from "./router.js";
import { registerStores } from "./components/stores.js";
import { registerTasksView } from "./components/tasksView.js";
import { registerLogsView } from "./components/logsView.js";
import { registerLogDetailView } from "./components/logDetailView.js";
import { registerTaskForm } from "./components/taskForm.js";
import { registerCronInput } from "./components/cronInput.js";
import { registerCronBuilder } from "./components/cronBuilder.js";

window.Alpine = Alpine;

const api = new ApiClient("");
const taskService = new TaskService(api);
const logService = new LogService(api);
const cronParser = new CronParser(window.cronstrue);

const router = new HashRouter(route => Alpine.store("app").setRoute(route));

registerStores(router);
registerTasksView(taskService);
registerLogsView(logService);
registerLogDetailView(logService);
registerTaskForm(taskService);
registerCronInput(cronParser);
registerCronBuilder();

Alpine.start();

router.start();
const monitor = new ConnectionMonitor(api, ok => Alpine.store("app").setConnected(ok));
monitor.start();
