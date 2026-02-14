import logging
import sys
import httpx
import json
from loguru import logger
from app.config import Settings

# Intercept standard logging
class InterceptHandler(logging.Handler):
    def emit(self, record):
        # Get corresponding Loguru level if it exists
        try:
            level = logger.level(record.levelname).name
        except ValueError:
            level = record.levelno

        # Find caller from where originated the logged message
        frame, depth = logging.currentframe(), 2
        while frame and frame.f_code.co_filename == logging.__file__:
            frame = frame.f_back
            depth += 1

        logger.opt(depth=depth, exception=record.exc_info).log(
            level, record.getMessage()
        )

class APIHandler(logging.Handler):
    def __init__(self, api_url: str, min_level: str = "ERROR"):
        super().__init__()
        self.api_url = api_url.rstrip('/')
        self.min_level_no = logging.getLevelName(min_level.upper())
        self.client = httpx.Client()
        self.client.timeout = 2.0
        logger.disable("httpx")

    def emit(self, record: logging.LogRecord):
        if record.levelno < self.min_level_no:
            return # Filter logs by level

        # Avoid sending logs about the API handler itself or httpx
        if record.name == __name__ or record.name.startswith("httpx"):
            return

        try:
            payload = {
                "level": record.levelname,
                "message": self.format(record),
                "source": "backend",
            }
            
            response = self.client.post(f"{self.api_url}/api/v1/logs/", json=payload)
            response.raise_for_status()
        except httpx.RequestError as exc:
            # Log to stderr if API sending fails
            print(f"ERROR: Could not send log to API at {self.api_url} - {exc}", file=sys.stderr)
        except httpx.HTTPStatusError as exc:
            print(f"ERROR: Log API responded with {exc.response.status_code} - {exc.response.text}", file=sys.stderr)
        except Exception as e:
            print(f"ERROR: An unexpected error occurred in APIHandler: {e}", file=sys.stderr)


def setup_logging(settings: Settings, log_to_api_min_level: str = None):
    """
    Set up structured logging for the application.
    """
    logger.remove()
    logger.add(
        sys.stdout,
        colorize=True,
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level: <8}</level> | <cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>",
        level="INFO"
    )
    logger.add(
        "logs/app.log",
        rotation="10 MB",
        retention="30 days",
        enqueue=True,
        serialize=True,
        format="{time} {level} {name}:{function}:{line} {message}",
        level="DEBUG"
    )

    # Add API handler if enabled
    if log_to_api_min_level:
        api_handler = APIHandler(settings.api_url, log_to_api_min_level)
        logging.getLogger().addHandler(api_handler)
        logger.info(f"API logging enabled for levels >= {log_to_api_min_level}")


    logging.basicConfig(handlers=[InterceptHandler()], level=0)
    logging.getLogger("uvicorn.access").handlers = [InterceptHandler()]
    logging.getLogger("uvicorn.error").handlers = [InterceptHandler()]
    logging.getLogger("sqlalchemy.engine").handlers = [InterceptHandler()]
    logging.getLogger("sqlalchemy.pool").handlers = [InterceptHandler()]
