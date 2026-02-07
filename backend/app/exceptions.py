from fastapi import HTTPException, status

class APIError(HTTPException):
    """Base class for API-related exceptions."""
    def __init__(self, status_code: int, detail: str, headers: dict = None):
        super().__init__(status_code=status_code, detail=detail, headers=headers)

class DatabaseError(APIError):
    """Custom exception for database-related errors."""
    def __init__(self, detail: str = "Database error", status_code: int = status.HTTP_500_INTERNAL_SERVER_ERROR):
        super().__init__(status_code=status_code, detail=detail)

class CacheError(APIError):
    """Custom exception for cache-related errors."""
    def __init__(self, detail: str = "Cache error", status_code: int = status.HTTP_500_INTERNAL_SERVER_ERROR):
        super().__init__(status_code=status_code, detail=detail)

class CollectorError(APIError):
    """Custom exception for metrics collector-related errors."""
    def __init__(self, detail: str = "Metrics collector error", status_code: int = status.HTTP_500_INTERNAL_SERVER_ERROR):
        super().__init__(status_code=status_code, detail=detail)

class MetricsNotFoundError(APIError):
    """Custom exception for when metrics are not found."""
    def __init__(self, detail: str = "Metrics not found", status_code: int = status.HTTP_404_NOT_FOUND):
        super().__init__(status_code=status_code, detail=detail)

class LogNotFoundError(APIError):
    """Custom exception for when a log entry is not found."""
    def __init__(self, detail: str = "Log entry not found", status_code: int = status.HTTP_404_NOT_FOUND):
        super().__init__(status_code=status_code, detail=detail)
