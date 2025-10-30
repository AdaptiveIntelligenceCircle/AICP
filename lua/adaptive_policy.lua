AdaptivePolicy = {}

-- handshake Policy 

function AdaptivePolicy.onHandshakeRequest(peer)
    if peer.trust < 0.5 then 
        return false, "Rejected: Low trust"
    end
    
    if peer.role == "external" and peer.token_expired then
        return false , "Rejected : Token expired"
    end 
    return true , "Accepted"
    
end

-- 2 . Message Routing Policy..
function AdaptivePolicy.onMessageRoute(message, channel)
    if message.priority == "Critical" then 
        return "core"
    elseif message.source == "ai_driver" then 
        return "driver_sync"
    else 
        return channel or "default"
    end 
end 

-- 3. Sync Merge Policy.... 
function AdaptivePolicy.onSyncDecision(localState, remoteState)
    -- prefer state with higher trust or newer timestamp.. 
    if remoteState.trust > localState.trust then 
        return remoteState
    elseif remoteState.version > localState.version then 
        return remoteState
    else 
        return localState
    end 
end 

--- Adaptive Threshold ----

AdaptivePolicy.trust_threshold = 0.6 
AdaptivePolicy.max_sync_delay = 500 -- ms 
AdaptivePolicy.enable_debug_log = true 

return AdaptivePolicy
