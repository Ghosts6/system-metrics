from datetime import datetime
from typing import Optional, List
from pydantic import BaseModel, Field, ConfigDict


class SystemMetricsBase(BaseModel):
    """Base schema for system metrics."""
    cpu_percent: float = Field(..., ge=0, le=100, description="CPU usage percentage")
    cpu_count: int = Field(..., gt=0, description="Number of CPU cores")
    cpu_freq_current: Optional[float] = Field(None, description="Current CPU frequency in MHz")
    cpu_freq_min: Optional[float] = Field(None, description="Minimum CPU frequency in MHz")
    cpu_freq_max: Optional[float] = Field(None, description="Maximum CPU frequency in MHz")
    
    memory_total: int = Field(..., ge=0, description="Total memory in bytes")
    memory_available: int = Field(..., ge=0, description="Available memory in bytes")
    memory_used: int = Field(..., ge=0, description="Used memory in bytes")
    memory_percent: float = Field(..., ge=0, le=100, description="Memory usage percentage")
    
    disk_total: int = Field(..., ge=0, description="Total disk space in bytes")
    disk_used: int = Field(..., ge=0, description="Used disk space in bytes")
    disk_free: int = Field(..., ge=0, description="Free disk space in bytes")
    disk_percent: float = Field(..., ge=0, le=100, description="Disk usage percentage")
    
    network_bytes_sent: Optional[int] = Field(None, ge=0, description="Network bytes sent")
    network_bytes_recv: Optional[int] = Field(None, ge=0, description="Network bytes received")
    
    hostname: Optional[str] = Field(None, description="System hostname")
    platform: Optional[str] = Field(None, description="Operating system platform")
    uptime_seconds: Optional[float] = Field(None, description="System uptime in seconds")


class SystemMetricsCreate(SystemMetricsBase):
    """Schema for creating system metrics."""
    pass


class SystemMetricsResponse(SystemMetricsBase):
    id: int
    timestamp: datetime
    
    model_config = ConfigDict(from_attributes=True)


class SystemMetricsListResponse(BaseModel):
    """Schema for paginated metrics list response."""
    items: List[SystemMetricsResponse]
    total: int
    page: int
    page_size: int
    pages: int


class SystemLogCreate(BaseModel):
    """Schema for creating system logs."""
    level: str = Field(..., description="Log level (INFO, WARNING, ERROR, etc.)")
    source: Optional[str] = Field(None, description="Log source")
    message: str = Field(..., description="Log message")
    log_metadata: Optional[str] = Field(None, description="Additional metadata as JSON string")


class SystemLogResponse(BaseModel):
    id: int
    timestamp: datetime
    level: str
    source: Optional[str]
    message: str
    log_metadata: Optional[str]
    
    model_config = ConfigDict(from_attributes=True)


class SystemLogListResponse(BaseModel):
    """Schema for paginated logs list response."""
    items: List[SystemLogResponse]
    total: int
    page: int
    page_size: int
    pages: int


class HealthResponse(BaseModel):
    """Schema for health check response."""
    status: str
    database: Optional[str] = None
    redis: Optional[str] = None
