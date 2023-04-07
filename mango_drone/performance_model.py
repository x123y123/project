import time
from typing import Any, Callable, List, TypeVar

F = TypeVar("F", bound=Callable[..., Any])

def performance_monitor(func: F) -> F:
    def wrapper(*args: Any, **kwargs: Any) -> Any:
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()
        execution_time = end_time - start_time
        print(f"{func.__name__} took {execution_time} seconds to execute.") 
        return result
    wrapper.execution_times: List[float] = []
    return wrapper
