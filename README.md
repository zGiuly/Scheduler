# C++ Cron Task Scheduler

A lightweight CLI task scheduler written in C++20 featuring MySQL persistence, an automatic configuration wizard, connection resilience with retry, thread safety, job execution timeout, and an HTTP API server for remote interactions.

## Key Features

- **Interactive Configuration**: Interactive setup CLI wizard on first start or if configuration is invalid.
- **Connection Resilience**: Automatic retry reconnect mechanism for database connection drops (up to 5 attempts, 2-second sleep).
- **Job Execution Timeout**: Configurable execution time limit (`job_timeout_ms`). Tasks exceeding this limit are aborted and logged as failed.
- **HTTP REST API**: Exposes endpoints to register tasks, list tasks, view logs, retrieve/delete logs, and manually trigger tasks.
- **Controller-Validator Architecture**: Decoupled routing layer with strict data validation.

## Prerequisites

- **CMake** (v3.15+)
- **MySQL Server**
- **vcpkg** — used to install OpenSSL automatically at configure time.

  Set the `VCPKG_ROOT` environment variable to your vcpkg installation directory:

  ```bash
  # Linux/macOS
  export VCPKG_ROOT=~/vcpkg

  # Windows (PowerShell)
  $env:VCPKG_ROOT = "C:\path\to\vcpkg"
  ```

  On **Windows with Visual Studio 2022**, vcpkg is bundled and no extra setup is needed.

  On **Linux/macOS**, install vcpkg if you don't have it:

  ```bash
  git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
  ~/vcpkg/bootstrap-vcpkg.sh
  export VCPKG_ROOT=~/vcpkg
  ```

  OpenSSL and any other dependencies declared in `vcpkg.json` are installed automatically during CMake configure.

## Build Instructions

Configure and build the project using CMake:

```bash
cmake -B out/build/x64-debug --preset x64-debug
cmake --build out/build/x64-debug
```

*Header-only dependencies (`cpp-httplib` and `nlohmann/json`) are fetched automatically via CMake FetchContent. OpenSSL is installed via vcpkg on first configure.*

## How to Run

1. Navigate to the build output directory:
   ```bash
   cd out/build/x64-debug
   ```
2. Start the executable:
   ```bash
   .\Scheduler.exe
   ```
3. Complete the interactive wizard in your terminal. It will save settings to `config.json`.

## Web UI Control Panel

The C++ server automatically hosts the static frontend files. Open `http://localhost:8080/` (or your configured HTTP port) in your browser to access the dashboard. It allows you to:
- Monitor and manage active tasks.
- Create new tasks using a modal form.
- Edit existing tasks with a dedicated form.
- Trigger tasks manually.
- View execution history and delete logs.

## HTTP REST API Endpoints

Once running, the scheduler listens on port `8080` (or your configured port).

### Tasks API
- `POST /api/tasks` - Register a new cron task.
  - Body:
    ```json
    {
      "project": "MyProject",
      "name": "AuditTask",
      "description": "Trigger daily auditing",
      "command": "http://localhost:3000/api/audit",
      "cron": "0 0 * * *",
      "type": "async",
      "oneshot": false
    }
    ```
- `GET /api/tasks` - List all tasks (including disabled ones).
- `POST /api/tasks/<id>/trigger` - Manually trigger a task by ID.
- `PUT /api/tasks/<id>` - Modify an existing task by ID (supports partial updates).
  - Body:
    ```json
    {
      "project": "MyProject-Updated",
      "name": "AuditTask-Updated",
      "description": "Updated description",
      "command": "http://localhost:3000/api/audit/v2",
      "cron": "*/5 * * * *",
      "type": "sync",
      "oneshot": false,
      "disabled": false
    }
    ```

### Logs API
- `GET /api/logs` - List all execution logs.
- `GET /api/logs/<id>` - Retrieve a specific log by ID.
- `DELETE /api/logs` - Delete multiple logs.
  - Body (JSON Array):
    ```json
    [1, 2, 3]
    ```
  - Or Body (JSON Object):
    ```json
    {
      "ids": [1, 2, 3]
    }
    ```
